//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Context.h"
#include "lace/core/Diagnostics.h"
#include "lace/core/ThreadPool.h"
#include "lace/core/Options.h"
#include "lace/lexer/Lexer.h"
#include "lace/lexer/TokenStream.h"
#include "lace/parser/Parser.h"
#include "lace/tools/Files.h"
#include "lace/tree/NameResolution.h"
#include "lace/tree/Rib.h"
#include "lace/tree/Defn.h"
#include "lace/tree/Codegen.h"
#include "lace/tree/Printer.h"
#include "lace/tree/SemanticAnalysis.h"
#include "lace/tree/SymbolAnalysis.h"

#include "lir/analysis/AMD64LoweringPass.h"
#include "lir/machine/AsmWriter.h"
#include "lir/machine/Machine.h"
#include "lir/machine/Printer.h"
#include "lir/machine/RegisterAnalysis.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define LACE_VERSION_MAJOR 1
#define LACE_VERSION_MINOR 0

using namespace lace;

using namespace std::chrono;

using Timestamp = time_point<high_resolution_clock>;

/// A mapping between the absolute path of an input file and its parsed AST.
static std::string g_STL = "/home/lovelace/stl";

static inline Timestamp get_time() {
    return high_resolution_clock::now();
}

int32_t main(int32_t argc, char* argv[]) {
    Options options = {};
    options.output = "main";
    options.opt = Options::OptLevel::Default;
    options.threads = 1;

    options.debug = true;
    options.link = true;
    options.multithread = true;
    options.stl = true;
    options.verbose = true;
    options.version = true;
    options.dump_ast = true;
    options.dump_lir = true;
    options.dump_mir = true;

    log::direct(std::cout);

    std::vector<std::string> files = {
        "/root/lace/stl/string.lace",
        "/root/lace/stl/mem.lace",
        "/root/lace/stl/linux.lace",
        "/root/lace/stl/index.lace",
    };

    for (int32_t i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-b") {
            options.verbose = true;
        } else if (arg == "-g") {
            options.debug = true;
        } else if (arg == "-l") {
            options.link = true;
        } else if (arg == "-v") {
            log::note("version: " + std::to_string(LACE_VERSION_MAJOR) + "." + 
                std::to_string(LACE_VERSION_MINOR));
        } else if (arg == "-Od") {
            options.opt = Options::OptLevel::Default;
        } else if (arg == "-Oa") {
            options.opt = Options::OptLevel::Aggressive;
        } else if (arg == "-Os") {
            options.opt = Options::OptLevel::Space;
        } else if (arg == "-st") {
            options.multithread = false;
        } else if (arg == "-stl") {
            options.stl = true;
        } else if (arg == "-no-stl") {
            options.stl = false;
        } else if (arg == "-dump-ast") {
            options.dump_ast = true;
        } else if (arg == "-dump-lir") {
            options.dump_lir = true;
        } else if (arg == "-dump-mir") {
            options.dump_mir = true;
        } else if (arg == "-j") {
            if (i + 1 == argc)
                log::fatal("expected number after -j");

            int32_t threads = std::stoi(argv[++i]);
            if (threads <= 0)
                log::fatal("thread count must be a positive number, got " + std::to_string(threads));

            options.threads = static_cast<uint32_t>(threads);
        } else if (arg == "-o") {
            if (i + 1 == argc)
                log::fatal("expected filename after -o");

            options.output = argv[++i];
        } else {
            if (arg.size() < 4 || arg.substr(arg.size() - 5) != ".lace")
                log::fatal("expected source file ending with \".lace\", got " + arg);

            bool dupe = false;
            std::string path = std::filesystem::absolute(arg).string();
            for (const std::string& file : files) {
                if (file == path) {
                    dupe = true;
                    break;
                }
            }

            if (!dupe)
                files.push_back(path);
        }
    }

    if (files.empty())
        log::fatal("no input files");

    Timestamp start = get_time();

    Context context(options);

    if (options.multithread) {
        const uint32_t supported_threads = std::thread::hardware_concurrency(); 

        if (options.threads == 1) {
            // If no -j was provided, then take the larger of 1 and the 
            // detected thread count.
            options.threads = std::max(1u, supported_threads);
        } else {
            // A -j was provided, but if it's larger than the number of threads 
            // supported, then use only what's available.
            options.threads = std::min(options.threads, supported_threads);
        }

        // No point in us using more threads than there are files.
        options.threads = std::min(options.threads, 
            static_cast<uint32_t>(files.size()));

        // Skip multithreading if we only have 1 thread available to us.
        if (options.threads == 1)
            options.multithread = false;
    }

    ThreadPool* tpool = nullptr;
    if (options.multithread) {
        tpool = new ThreadPool(options.threads);
        assert(tpool);
    }

    auto parse_file = [&context](const std::string& file) {
        const Timestamp pstart = get_time();

        std::string contents;
        if (!read_file(file, contents))
            log::flush();

        TokenStream tstream;
        Lexer lexer(contents, file);
        if (!lexer.lex(tstream))
            log::flush();

        Parser parser(tstream, file);
        Rib* rib = parser.parse();
        assert(rib);

        if (!context.add_rib(rib))
            log::error("rib '" + rib->name() + "' has multiple definitions");

        if (context.options().verbose) {
            duration<double> dur = get_time() - pstart;

            std::stringstream ss;
            ss << std::format("{}: Finished parsing\n-- took {}\n", 
                file, dur);
            
            std::cout << ss.str();
        }
    };

    if (options.multithread) {
        for (const std::string& file : files) {
            tpool->push([&file, &context, &parse_file] { 
                parse_file(file); 
            });
        }

        tpool->wait();
    } else for (const std::string& file : files) {
        parse_file(file);
    }

    log::flush();

    for (const auto& [name, rib] : context.ribs()) {
        const Timestamp pstart = get_time();

        SymbolAnalysis syma(context);
        rib->accept(syma);

        if (context.options().verbose) {
            duration<double> dur = get_time() - pstart;
            std::cout << std::format("{}: Finished symbol analysis\n-- took {}\n", 
                rib->path(), dur);
        }
    }

    log::flush();

    for (const auto& [name, rib] : context.ribs()) {
        const Timestamp pstart = get_time();

        NameResolution nres(context);
        rib->accept(nres);

        if (context.options().verbose) {
            duration<double> dur = get_time() - pstart;
            std::cout << std::format("{}: Finished name resolution\n-- took {}\n", 
                rib->path(), dur);
        }
    }

    log::flush();

    for (const auto& [name, rib] : context.ribs()) {
        const Timestamp pstart = get_time();

        SemanticAnalysis sema(context);
        rib->accept(sema);

        if (context.options().verbose) {
            duration<double> dur = get_time() - pstart;
            std::cout << std::format("{}: Finished semantic analysis\n-- took {}\n", 
                rib->path(), dur);
        }
    }

    log::flush();

    if (context.options().dump_ast) {
        for (const auto& [name, rib] : context.ribs()) {
            std::ofstream out(rib->path() + ".ast");
            if (!out || !out.is_open())
                log::fatal("failed to open file: " + rib->path() + ".ast");

            Printer printer(context, out);
            rib->accept(printer);

            out.close();
        }

        log::flush();
    }

    // Collect all ribs under their root identifiers. 
    std::unordered_map<std::string, std::unordered_set<Rib*>> translations = {};
    for (const auto& [name, rib] : context.ribs()) {
        // Determine the root of the rib name. This is the name before the 
        // first path '::' delimiter, if there is one.
        std::size_t dpos = name.find_first_of("::");
        std::string root = name;
        if (dpos != std::string::npos)
            root = name.substr(0, dpos);

        if (translations.contains(root)) {
            translations[root].insert(rib);
        } else {
            translations.emplace(root, std::unordered_set<Rib*>({ rib }));
        }
    }

    lir::Machine mach(lir::Machine::Linux);

    for (const auto& [root, ribs] : translations) {
        Timestamp cstart = get_time();

        lir::CFG graph(mach, "");

        Codegen codegen(context, graph, mach);
        
        for (Rib* rib : ribs)
            rib->accept(codegen);

        if (context.options().verbose) {
            duration<double> dur = get_time() - cstart;
            std::cout << std::format("{}: Finished code generation\n-- took{}\n", root, dur);
        }

        if (context.options().dump_lir) {
            std::ofstream file(root + ".lir");
            if (!file || !file.is_open())
                log::fatal("failed to open: " + root + ".s");
            
            graph.print(file);
            file.close();
        }

        Timestamp lstart = get_time();

        lir::MachineObject mobj(mach);
        lir::AMD64LoweringPass lowering(graph, mobj);
        lowering.run();

        if (context.options().verbose) {
            duration<double> dur = get_time() - lstart;
            std::cout << std::format("{}: Finished lowering\n-- took {}\n", root, dur);
        }

        if (context.options().dump_mir) {
            std::ofstream mir(root + ".mir");
            if (!mir || !mir.is_open())
                log::fatal("failed to open: " + root + ".mir");

            lir::Printer printer(mobj);
            printer.run(mir);
            mir.close();
        }

        Timestamp rstart = get_time();

        lir::RegisterAnalysis rega(mobj);
        rega.run();

        if (context.options().verbose) {
            duration<double> dur = get_time() - rstart;
            std::cout << std::format("{}: Finished register analysis\n-- took{}\n", root, dur);
        }

        std::ofstream as(root + ".s");
        if (!as || !as.is_open())
            log::fatal("failed to open: " + root + ".s");

        lir::AsmWriter writer(mobj);
        writer.run(as);
        as.close();

        const std::string assembler = std::format("as {}.s -o {}.o", root, root);
        std::system(assembler.c_str());
    }

    /*
    if (context.options().link) {
        std::string linker = std::format("ld -o {} ", options.output);

        for (const auto& [root, ribs] : translations)
            linker += std::format("{}.o ", root);
        
        if (options.stl)
            linker += std::format("{}/rt.o", g_STL);

        std::system(linker.c_str());
    }
    */

    if (options.verbose) {
        duration<double> dur = get_time() - start;
        std::cout << std::format("Finished all compilation procedures.\n-- took {}\n", dur);
    }

    return 0;
}
