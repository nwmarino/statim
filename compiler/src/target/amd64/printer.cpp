#include "machine/basicblock.hpp"
#include "machine/function.hpp"
#include "machine/inst.hpp"
#include "machine/operand.hpp"
#include "target/amd64.hpp"
#include <string>

using namespace stm;
using namespace stm::amd64;

static void print_operand(std::ostream& os, const MachineOperand& MO) {

}

static void print_inst(std::ostream& os, const MachineInst& MI) {

}

static void print_block(std::ostream& os, const MachineBasicBlock& MBB) {
    os << "bb" << MBB.position() << ":\n";

    for (auto inst : MBB.insts()) {
        print_inst(os, inst);
        os << "\n";
    }
}

static void print_function(std::ostream& os, const MachineFunction& MF) {
    os << MF.get_name() << ":\n";

    for (auto curr = MF.front(); curr; curr = curr->next())
        print_block(os, *curr);
}

void amd64::Printer::run(std::ostream& os) const {
    os << "MACHINE CODE amd64\n"
       << "   " << std::to_string(m_frame.functions().size()) << " functions\n"
       << "\n";

    for (auto function : m_frame.functions()) {
        print_function(os, *function);
        os << "\n";
    }
}
