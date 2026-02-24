//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.h"
#include "lace/core/ThreadPool.h"
#include "lace/core/Options.h"
#include "lace/lexer/Lexer.h"
#include "lace/lexer/TokenStream.h"
#include "lace/parser/Parser.h"
#include "lace/tools/Files.h"
#include "lace/tree/AST.h"
#include "lace/tree/Defn.h"
#include "lace/tree/LIRCodegen.h"
#include "lace/tree/Printer.h"
#include "lace/tree/SemanticAnalysis.h"
#include "lace/tree/SymbolAnalysis.h"
#include "lace/tree/TypeResolution.h"

#include "lir/analysis/AMD64LoweringPass.h"
#include "lir/machine/AsmWriter.h"
#include "lir/machine/Machine.h"
#include "lir/machine/Printer.h"
#include "lir/machine/RegisterAnalysis.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define LACE_VERSION_MAJOR 1
#define LACE_VERSION_MINOR 0

using namespace lace;

using namespace std::chrono;
using namespace std::filesystem;

using Asts = std::unordered_set<AST*>;
using DepTable = std::unordered_map<AST*, Asts>;
using FileTable = std::unordered_map<std::string, AST*>;

using Timestamp = time_point<high_resolution_clock>;

struct InputFile final {
    std::string file;
    AST* ast;

    InputFile(const std::string& file, AST* ast = nullptr) 
      : file(file), ast(ast) {}
};

/// A mapping between the absolute path of an input file and its parsed AST.
static FileTable g_files = {};
static std::string g_STL = "/home/lovelace/stl";

static inline Timestamp get_time() {
    return high_resolution_clock::now();
}

/// Setup |g_files| based on the set of given |asts| and their respective
/// input files.
void setup_file_table(const Asts& asts) {
    g_files.reserve(asts.size());
    for (AST* ast : asts) {
        g_files.emplace(
            std::filesystem::absolute(ast->get_file()).string(), ast); 
    }
}

/// Compute the dependency order and a dependency table for each file in the 
/// list of |asts|.
///
/// This function produces an ordering of the files without cycles to 
/// |ordering|, s.t. any given file relies on only the ones which come 
/// before it in the set. This means the result is a valid ordering in which to 
/// sequentially perform name analysis on each syntax tree.
///
/// Moreover, as it computes dependencies, it saves them to |deps|.
void computeDependencies(const Asts& asts, Asts& ordering, DepTable& deps) {
    for (AST* ast : asts) {
        path parent = absolute(ast->get_file()).parent_path();
    
        for (Defn* defn : ast->defns()) {
            LoadDefn* load = dynamic_cast<LoadDefn*>(defn);
            if (!load)
                continue;

            // Find the canonical path for the target file.
            path target = parent / load->path();
            target = weakly_canonical(target);

            auto it = g_files.find(target.string());
            if (it != g_files.end()) {
                deps[ast].insert(it->second);
                load->set_path(target.string());
            } else {
                log::fatal("unresolved file: " + target.string(), 
                    log::Span(ast->get_file(), load->span()));
            }
        }
    }

    Asts visited = {};
    Asts visiting = {};

    std::function<void(AST*)> dfs = [&](AST* ast) {
        if (visited.count(ast)) 
            return;

        if (visiting.count(ast))
            log::fatal("cyclic dependency found", 
                log::Location(ast->get_file(), { 1, 1 }));
        
        visiting.insert(ast);
        
        for (AST* dep : deps[ast])
            dfs(dep);

        visiting.erase(ast);
        visited.insert(ast);
        ordering.insert(ast);
    };

    for (AST* ast : asts)
        dfs(ast);
}

void merge_namespace(AST* ast, Scope* dest, SpaceDefn* incoming) {
    // Check for an existing namespace in the |dest| scope with the same name.
    SpaceDefn* existing = dest->get_namespace(incoming->name());
    if (!existing) {
        // If an existing namespace does not exist, just try to add the 
        // namespace as is.
        if (!dest->add(incoming)) {
            log::fatal("failed to load namespace, name already exists: " 
                + incoming->name(), log::Location(ast->get_file(), { 1, 1 }));
        }
        
        return;
    }

    // An existing namespace with the same name as |incoming| exists.
    // So, we must recursively merge all definitions in |incoming| with 
    // whatever may exist in the |dest| scope.

    for (auto& [name, defn] : incoming->scope()->defns()) {
        // Skip private definitions.
        if (!defn->has_rune(Rune::Kind::Public))
            continue;

        if (defn->origin() != incoming->origin())
            continue;

        if (SpaceDefn* nspace = dynamic_cast<SpaceDefn*>(defn)) {
            // If we have to import a nested namespace, then merge it too.
            merge_namespace(ast, existing->scope(), nspace);
        } else if (!existing->scope()->add(defn)) {
            log::fatal("name-wise conflict during load: " + name,
                log::Location(ast->get_file(), { 1, 1 }));
        }
    }
}

/// Resolve the dependent symbols for each tree in |asts|, based on their
/// dependencies defined in |deps|. 
/// Assumes that |asts| contains syntax trees in their dependency order.
void resolveDependencies(Options& options, const Asts& asts, const DepTable& deps) {
    for (AST* ast : asts) {
        Asts dep_list = deps.at(ast);
        std::vector<NamedDefn*> symbols = {};

        // For each dependency, fetch all of its public, named definitions.
        for (AST* dep : dep_list) {
            for (Defn* defn : dep->defns()) {
                NamedDefn* symbol = dynamic_cast<NamedDefn*>(defn);
                if (symbol && symbol->has_rune(Rune::Kind::Public))
                    symbols.push_back(symbol);
            }
        }

        Scope* scope = ast->scope();
        for (NamedDefn* symbol : symbols) {
            if (SpaceDefn* nspace = dynamic_cast<SpaceDefn*>(symbol)) {
                merge_namespace(ast, ast->scope(), nspace);
            } else {
                bool res = scope->add(symbol);
                if (!res) {
                    log::fatal("name-wise conflict with an existing definition: " 
                        + symbol->name(), log::Location(ast->get_file(), { 1, 1 }));
                }
            }

            ast->defns().push_back(symbol);
        }

        const Timestamp time_namea_start = get_time();

        TypeResolution type_res(options);
        ast->accept(type_res);

        if (options.verbose) {
            duration<double> dur = get_time() - time_namea_start;
            std::cout << std::format("{}: Finished type resolution\n-- took {}\n",
                ast->get_file(), dur);
        }
    }
}

void drive_lir_backend(const Options &options, const Asts &asts) {
    lir::Machine mach(lir::Machine::Linux);

    for (AST *ast : asts) {
        Timestamp time_cgn_start = get_time();

        lir::CFG cfg(mach, ast->get_file());        

        LIRCodegen codegen(options, ast, cfg);
        codegen.run();

        Timestamp time_cgn_end = get_time();
        if (options.verbose) {
            duration<double> dur = time_cgn_end - time_cgn_start;
            std::cout << std::format("{}: Finished code generation\n-- took {}\n", 
                ast->get_file(), dur);
        }

        if (options.dump_lir) {
            std::ofstream file(ast->get_file() + ".lir");
            if (!file || !file.is_open())
                log::fatal("failed to open: " + ast->get_file() + ".s");
            
            cfg.print(file);
            file.close();
        }

        Timestamp time_lower_start = get_time();

        lir::MachineObject obj(mach);

        lir::AMD64LoweringPass lowering(cfg, obj);
        lowering.run();

        Timestamp time_lower_end = get_time();
        if (options.verbose) {
            duration<double> dur = time_lower_end - time_lower_start;
            std::cout << std::format("{}: Finished lowering\n-- took {}\n", 
                ast->get_file(), dur);
        }

        if (options.dump_mir) {
            std::ofstream mir(ast->get_file() + ".mir");
            if (!mir || !mir.is_open())
                log::fatal("failed to open: " + ast->get_file() + ".mir");

            lir::Printer printer(obj);
            printer.run(mir);
            mir.close();
        }

        Timestamp time_rega_start = get_time();

        lir::RegisterAnalysis rega(obj);
        rega.run();

        Timestamp time_rega_end = get_time();
        if (options.verbose) {
            duration<double> dur = time_rega_end - time_rega_start;
            std::cout << std::format("{}: Finished register analysis\n-- took {}\n", 
                ast->get_file(), dur);
        }

        if (options.dump_mir) {
            std::ofstream rmir(ast->get_file() + ".rmir");
            if (!rmir || !rmir.is_open())
                log::fatal("failed to open: " + ast->get_file() + ".rmir");

            lir::Printer printer(obj);
            printer.run(rmir);
            rmir.close();
        }

        std::ofstream as(ast->get_file() + ".s");
        if (!as || !as.is_open())
            log::fatal("failed to open: " + ast->get_file() + ".s");

        lir::AsmWriter writer(obj);
        writer.run(as);
        as.close();

        const std::string assembler = std::format(
            "as {}.s -o {}.o", 
            ast->get_file(), 
            ast->get_file()
        );

        std::system(assembler.c_str());
    }

    if (options.link) {
        std::string linker = std::format("ld -o {} ", options.output);

        for (AST* ast : asts)
            linker += std::format("{}.o ", ast->get_file());
        
        if (options.stl)
            linker += std::format("{}/rt.o", g_STL);

        std::system(linker.c_str());
    }
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

    std::vector<InputFile> files = {
        InputFile("/home/lovelace/stl/string.lace"),
        InputFile("/home/lovelace/stl/mem.lace"),
        InputFile("/home/lovelace/stl/linux.lace"),
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
                log::fatal("thread count must be a positive number, got " 
                    + std::to_string(threads));

            options.threads = static_cast<uint32_t>(threads);
        } else if (arg == "-o") {
            if (i + 1 == argc)
                log::fatal("expected filename after -o");

            options.output = argv[++i];
        } else {
            if (arg.size() < 4 || arg.substr(arg.size() - 5) != ".lace")
                log::error("expected source file ending with \".lace\", got " + arg);

            bool dupe = false;
            std::string path = absolute(arg).string();
            for (const InputFile& f : files) {
                if (f.file == path) {
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

    log::flush();

    Timestamp start = get_time();

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
    }

    ThreadPool *pool = nullptr;
    if (options.multithread) {
        pool = new ThreadPool(options.threads);
        assert(pool);
    }

    if (options.multithread && options.threads > 1) {
        assert(pool);

        for (InputFile &f : files) {
            pool->push([&f, options] {
                Timestamp parse_start = get_time();

                std::string contents;
                if (!read_file(f.file, contents))
                    log::flush();

                TokenStream stream;
                Lexer lexer(contents, f.file);
                if (!lexer.lex(stream))
                    log::flush();

                Parser parser(stream, f.file);
                f.ast = parser.parse();
                assert(f.ast);

                if (options.verbose) {
                    duration<double> dur = get_time() - parse_start;

                    std::stringstream ss;
                    ss << std::format("{}: Finished parsing\n-- took {}\n", 
                        f.file, dur);
                    
                    std::cout << ss.str();
                }
            });
        }

        pool->wait();
    } else for (InputFile &f : files) {
        Timestamp parse_start = get_time();

        std::string contents;
        if (!read_file(f.file, contents))
            log::flush();

        TokenStream stream;
        Lexer lexer(contents, f.file);
        if (!lexer.lex(stream))
            log::flush();

        Parser parser(stream, f.file);
        f.ast = parser.parse();
        assert(f.ast);
        
        if (options.verbose) {
            duration<double> dur = get_time() - parse_start;

            std::stringstream ss;
            ss << std::format("{}: Finished parsing\n-- took {}\n", f.file, dur);
            
            std::cout << ss.str();
        }
    }

    log::flush();

    Asts asts = {};
    asts.reserve(files.size());
    for (InputFile &f : files)
        asts.insert(f.ast);

    setup_file_table(asts);

    Asts ordering = {};
    DepTable deps = {};
    ordering.reserve(asts.size());
    deps.reserve(asts.size());

    computeDependencies(asts, ordering, deps);
    resolveDependencies(options, ordering, deps);

    // Perform symbol analysis on each syntax tree.
    for (AST* ast : asts) {
        const Timestamp syma_start = get_time();

        SymbolAnalysis symbol_analysis(options);
        ast->accept(symbol_analysis);

        if (options.verbose) {
            duration<double> dur = get_time() - syma_start;
            std::cout << std::format("{}: Finished symbol analysis\n-- took {}\n", 
                ast->get_file(), dur);
        }
    }

    log::flush();

    // Perform semantic analysis on each syntax tree.
    for (AST *ast : asts) {
        const Timestamp sema_start = get_time();

        SemanticAnalysis semantic_analysis(options);
        ast->accept(semantic_analysis);

        if (options.verbose) {
            duration<double> dur = get_time() - sema_start;
            std::cout << std::format("{}: Finished semantic analysis\n-- took {}\n", 
                ast->get_file(), dur);
        }

        // AST is now considered valid, so print it if needbe.
        if (options.dump_ast) {
            std::ofstream out(ast->get_file() + ".ast");
            if (!out || !out.is_open())
                log::fatal("failed to open file: " + ast->get_file() + ".ast");

            Printer printer(options, out);
            ast->accept(printer);
            out.close();
        }
    }

    log::flush();

    drive_lir_backend(options, asts);

    for (AST* ast : asts)
        delete ast;

    asts.clear();
    files.clear();

    if (options.verbose) {
        duration<double> dur = get_time() - start;
        std::cout << std::format("Finished all compilation procedures.\n-- took {}\n", dur);
    }

    return 0;
}
