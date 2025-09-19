#include "siir/allocator.hpp"
#include "siir/cfg.hpp"
#include "siir/function.hpp"
#include "siir/machine_analysis.hpp"
#include "siir/machine_basicblock.hpp"
#include "siir/machine_function.hpp"
#include "siir/machine_object.hpp"
#include "x64/x64.hpp"
#include <iostream>

using namespace stm;
using namespace stm::siir;

//#define DEBUG_PRINT_RANGES

class LinearScan final {
    const MachineFunction& m_function;
    std::vector<LiveRange>& m_ranges;

    LiveRange& update_range(MachineRegister reg, RegisterClass cls, u32 pos) {
        // Attempt to find an existing range for |reg|.
        for (auto& range : m_ranges) {
            // The range has been killed, so we never update it.
            if (range.killed)
                continue;

            if (range.reg == reg) {
                range.end = pos;
                return range;
            }
        }

        // No existing range could be found, so we begin a new one.
        LiveRange range;
        range.reg = reg;
        range.start = range.end = pos;
        range.cls = cls;
        range.killed = false;

        if (reg.is_physical()) {
            range.alloc = reg;
        } else {
            range.alloc = MachineRegister::NoRegister;
        }

        m_ranges.push_back(range);
        return m_ranges.back();
    }

public:
    LinearScan(const MachineFunction& function, std::vector<LiveRange>& ranges)
        : m_function(function), m_ranges(ranges) {}

    LinearScan(const LinearScan&) = delete;
    LinearScan& operator = (const LinearScan&) = delete;

    ~LinearScan() = default;

    void run() {
        u32 position = 0;

        for (const auto* mbb = m_function.front(); mbb; mbb = mbb->next()) {
            for (const auto& mi : mbb->insts()) {
                for (const auto& mo : mi.operands()) {
                    if (!mo.is_reg() && !mo.is_mem())
                        continue;

                    MachineRegister reg;
                    RegisterClass cls;
                    if (mo.is_reg())
                        reg = mo.get_reg();
                    else if (mo.is_mem())
                        reg = mo.get_mem_base();

                    if (reg.is_physical()) {
                        cls = x64::get_class(
                            static_cast<x64::Register>(reg.id()));
                    } else {
                        // reg refers to a virtual register, whose
                        // information is stored in the parent function.
                        const auto& regi = m_function.get_register_info();
                        assert(regi.vregs.count(reg.id()) != 0);
                        cls = regi.vregs.at(reg.id()).cls;
                    }

                    LiveRange& range = update_range(reg, cls, position);
                    if (mo.is_reg() && mo.is_kill()) {
                        range.end = position;
                        range.killed = true;
                    }
                }

                ++position;
            }
        }
    }
};

class CallsiteAnalysis final {
    MachineFunction& m_function;
    const std::vector<LiveRange>& m_ranges;

public:
    CallsiteAnalysis(MachineFunction& function, 
                     const std::vector<LiveRange>& ranges)
        : m_function(function), m_ranges(ranges) {}

    CallsiteAnalysis(const CallsiteAnalysis&) = delete;
    CallsiteAnalysis& operator = (const CallsiteAnalysis&) = delete;

    ~CallsiteAnalysis() = default;
    
    void run() {
        u32 position = 0;
        for (auto* mbb = m_function.front(); mbb; mbb = mbb->next()) {
            std::vector<MachineInst> insts;
            insts.reserve(mbb->size());

            for (u32 i = 0; i < mbb->size(); ++position, ++i) {
                // TODO: Generalize for other targets.
                if (x64::is_call_opcode(static_cast<x64::Opcode>(mbb->insts().at(i).opcode()))) {
                    std::vector<MachineRegister> save = {};

                    for (auto& range : m_ranges) {
                        if (range.overlaps(position)) {
                            MachineRegister range_alloc = range.alloc;
                            if (x64::is_caller_saved(static_cast<x64::Register>(range.alloc.id())))
                                save.push_back(range.alloc);
                        }
                    }

                    for (auto& reg : save) {
                        MachineOperand op = MachineOperand::create_reg(
                            reg, 8, false);

                        insts.push_back({ x64::PUSH64, { op } });   
                    }

                    insts.push_back(mbb->insts().at(i));
                    
                    for (auto& reg : save) {
                        MachineOperand op = MachineOperand::create_reg(
                            reg, 8, true);

                        insts.push_back({ x64::POP64, { op } });
                    }
                } else {
                    insts.push_back(mbb->insts().at(i));
                }
            }

            mbb->insts() = insts;
        }
    }
};

CFGMachineAnalysis::CFGMachineAnalysis(CFG& cfg) : m_cfg(cfg) {}

void CFGMachineAnalysis::run(MachineObject& obj) {
    for (const auto& function : m_cfg.functions()) {
        // Empty functions should not be lowered, they should either be
        // resolved at link time or with some library.
        if (function->empty())
            continue;

        MachineFunction* mf = new MachineFunction(function, *obj.get_target());
        obj.functions().emplace(mf->get_name(), mf);

        for (auto curr = function->front(); curr; curr = curr->next())
            new MachineBasicBlock(curr, mf);

        switch (obj.get_target()->arch()) {
        case Target::x64: {
            x64::X64InstSelection isel { mf };
            isel.run();
            break;
        }

        default:
            assert(false && "unsupported architecture!");
        }
    }
}

FunctionRegisterAnalysis::FunctionRegisterAnalysis(MachineObject& obj)
    : m_obj(obj) {}

void FunctionRegisterAnalysis::run() {
    for (const auto& [name, function] : m_obj.functions()) {
        std::vector<LiveRange> ranges;
        
        LinearScan linscan { *function, ranges };
        linscan.run();

        TargetRegisters tregs;
        switch (m_obj.get_target()->arch()) {
        case Target::x64:
            tregs = x64::get_registers();
            break;

        default:
            assert(false && "unsupported architecture!");
        }

#ifdef DEBUG_PRINT_RANGES
        std::cerr << "Function '" << name << "' ranges:\n";
        for (auto& range : ranges) {
            if (range.reg.is_virtual()) {
                std::cerr << 'v' << range.reg.id() - MachineRegister::VirtualBarrier;
            } else {
                std::cerr << '%' << x64::to_string(static_cast<x64::Register>(
                    range.reg.id()), 8);
            }

            std::cerr << " [" << range.start << ", " << range.end << "]\n";
        }
#endif // DEBUG_PRINT_RANGES

        RegisterAllocator allocator { *function, tregs, ranges };
        allocator.run();

        FunctionRegisterInfo& regi = function->get_register_info();
        for (auto& range : ranges) {
            MachineRegister reg = range.reg;
            if (reg.is_physical())
                continue;

            regi.vregs[reg.id()].alloc = range.alloc;
        }

        /// TODO: Implement callsite analysis at this point, saving caller-
        /// saved registers that are live around callsites, and managing the
        /// spills that come with it.

        CallsiteAnalysis CAN { *function, ranges };
        CAN.run();
    }
}

MachineObjectPrinter::MachineObjectPrinter(MachineObject& obj) : m_obj(obj) {}

void MachineObjectPrinter::run(std::ostream& os) {
    switch (m_obj.get_target()->arch()) {
    case Target::x64: {
        x64::X64Printer printer { m_obj };
        printer.run(os);
        break;
    }

    default:
        assert(false && "unsupported architecture!");
    }
}

MachineObjectAsmWriter::MachineObjectAsmWriter(MachineObject& obj) 
    : m_obj(obj) {}

void MachineObjectAsmWriter::run(std::ostream& os) {
    switch (m_obj.get_target()->arch()) {
    case Target::x64: {
        x64::X64AsmWriter writer { m_obj };
        writer.run(os);
        break;
    }

    default:
        assert(false && "unsupported architecture!");
    }
}
