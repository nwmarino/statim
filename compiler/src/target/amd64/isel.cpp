#include "target/amd64.hpp"
#include "machine/function.hpp"
#include "machine/basicblock.hpp"
#include "machine/operand.hpp"
#include "machine/inst.hpp"

using namespace stm;
using namespace stm::amd64;

InstSelection::InstSelection(MachineFunction* function) 
    : m_function(function) {}

void InstSelection::run() {

}

amd64::Opcode InstSelection::flip_jcc(amd64::Opcode jcc) {

}

amd64::Opcode InstSelection::neg_jcc(amd64::Opcode jcc) {

}

amd64::Opcode InstSelection::flip_setcc(amd64::Opcode setcc) {

}

amd64::Opcode InstSelection::neg_setcc(amd64::Opcode setcc) {

}

MachineOperand InstSelection::lower(const Operand& operand) {

}

void InstSelection::select(Instruction* inst) {

}

void InstSelection::select_const(Instruction* inst) {

}

void InstSelection::select_move(Instruction* inst) {

}

void InstSelection::select_lea(Instruction* inst) {

}

void InstSelection::select_copy(Instruction* inst) {

}

void InstSelection::select_jump(Instruction* inst) {

}

void InstSelection::select_branch(Instruction* inst) {

}

void InstSelection::select_set(Instruction* inst) {

}

void InstSelection::select_return(Instruction* inst) {

}

void InstSelection::select_call(Instruction* inst) {

}

void InstSelection::select_arith(Instruction* inst) {

}

void InstSelection::select_crement(Instruction* inst) {

}

void InstSelection::select_neg(Instruction* inst) {

}

void InstSelection::select_not(Instruction* inst) {

}

void InstSelection::select_logic(Instruction* inst) {

}

void InstSelection::select_shift(Instruction* inst) {

}

void InstSelection::select_sext(Instruction* inst) {

}

void InstSelection::select_zext(Instruction* inst) {

}

void InstSelection::select_fext(Instruction* inst) {

}

void InstSelection::select_trunc(Instruction* inst) {

}

void InstSelection::select_ftrunc(Instruction *inst) {

}

void InstSelection::select_cvt(Instruction* inst) {

}

void InstSelection::select_cmp(Instruction* inst) {

}
