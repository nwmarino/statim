#ifndef SPBE_INSTRUCTION_H_
#define SPBE_INSTRUCTION_H_

#include "common.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct SpbeBasicBlock SpbeBasicBlock;

typedef enum SpbeOpcode {
    SPBE_OPCODE_NOP,
    SPBE_OPCODE_CONSTANT,
    SPBE_OPCODE_STRING,
    SPBE_OPCODE_LOAD,
    SPBE_OPCODE_STORE,
    SPBE_OPCODE_ACCESS_PTR,
    SPBE_OPCODE_SELECT,
    SPBE_OPCODE_BRANCH_IF,
    SPBE_OPCODE_JUMP,
    SPBE_OPCODE_PHI,
    SPBE_OPCODE_RETURN,
    SPBE_OPCODE_ABORT,
    SPBE_OPCODE_UNREACHABLE,
    SPBE_OPCODE_CALL,
    SPBE_OPCODE_IADD,
    SPBE_OPCODE_FADD,
    SPBE_OPCODE_ISUB,
    SPBE_OPCODE_FSUB,
    SPBE_OPCODE_IMUL,
    SPBE_OPCODE_FMUL,
    SPBE_OPCODE_SDIV,
    SPBE_OPCODE_UDIV,
    SPBE_OPCODE_FDIV,
    SPBE_OPCODE_SREM,
    SPBE_OPCODE_UREM,
    SPBE_OPCODE_AND,
    SPBE_OPCODE_OR,
    SPBE_OPCODE_XOR,
    SPBE_OPCODE_SHL,
    SPBE_OPCODE_SHR,
    SPBE_OPCODE_SAR,
    SPBE_OPCODE_NOT,
    SPBE_OPCODE_INEG,
    SPBE_OPCODE_FNEG,
    SPBE_OPCODE_SEXT,
    SPBE_OPCODE_ZEXT,
    SPBE_OPCODE_FEXT,
    SPBE_OPCODE_ITRUNC,
    SPBE_OPCODE_FTRUNC,
    SPBE_OPCODE_SI2FP,
    SPBE_OPCODE_UI2FP,
    SPBE_OPCODE_FP2SI,
    SPBE_OPCODE_FP2UI,
    SPBE_OPCODE_P2I,
    SPBE_OPCODE_I2P,
    SPBE_OPCODE_REINTERPRET,
    SPBE_OPCODE_CMPIEQ,
    SPBE_OPCODE_CMPINE,
    SPBE_OPCODE_CMPOEQ,
    SPBE_OPCODE_CMPONE,
    SPBE_OPCODE_CMPUNEQ,
    SPBE_OPCODE_CMPUNNE,
    SPBE_OPCODE_CMPSLT,
    SPBE_OPCODE_CMPSLE,
    SPBE_OPCODE_CMPSGT,
    SPBE_OPCODE_CMPSGE,
    SPBE_OPCODE_CMPULT,
    SPBE_OPCODE_CMPULE,
    SPBE_OPCODE_CMPUGT,
    SPBE_OPCODE_CMPUGE,
    SPBE_OPCODE_CMPOLT,
    SPBE_OPCODE_CMPOLE,
    SPBE_OPCODE_CMPOGT,
    SPBE_OPCODE_CMPOGE,
    SPBE_OPCODE_CMPUNLT,
    SPBE_OPCODE_CMPUNLE,
    SPBE_OPCODE_CMPUNGT,
    SPBE_OPCODE_CMPUNGE,
} SpbeOpcode;

typedef struct SpbeInstruction {
    SpbeUser            base;
    SpbeOpcode          opcode;
    u32                 result;
    u32                 data;
    SpbeBasicBlock*     pParent;
    SpbeInstruction*    pPrev;
    SpbeInstruction*    pNext;
} SpbeInstruction;

SPBE_API u32 spbe_opcode_is_cast(SpbeOpcode op);
SPBE_API u32 spbe_opcode_is_comparison(SpbeOpcode op);
SPBE_API u32 spbe_opcode_is_terminator(SpbeOpcode op);

//! Returns a string which represents the opcode |op|.
SPBE_API const char* 
spbe_opcode_to_string(SpbeOpcode op);

//! Destroy instruction |pInst| and detach it from any parent block.
SPBE_API void 
spbe_instr_destroy(SpbeInstruction* pInst);

//! Returns true if instruction |pInst| defines a value.
SPBE_API u32 
spbe_instr_is_def(SpbeInstruction* pInst);

//! Returns the value operand of instruction |pInst| at index |i|.
SPBE_API SpbeValue* 
spbe_instr_get_operand(const SpbeInstruction* pInst, u32 i);

//! Prepend the instruction |pInst| to basic block |pBlock|. This function
//! assumes that |pInst| does not currently have a parent basic block.
SPBE_API void 
spbe_instr_prepend_to(SpbeInstruction* pInst, SpbeBasicBlock* pBlock);

//! Append the instruction |pInst| to basic block |pBlock|. This function
//! assumes that |pInst| does not currently have a parent basic block.
SPBE_API void 
spbe_instr_append_to(SpbeInstruction* pInst, SpbeBasicBlock* pBlock);

//! Insert the instruction |pInst| immediately before |pTarget| in the parent
//! basic block of |pTarget|. This function assumes that |pInst| does not
//! already hav a parent basic block.
SPBE_API void 
spbe_instr_insert_before(SpbeInstruction* pInst, SpbeInstruction* pTarget);

//! Insert the instruction |pInst| immediately after |pTarget| in the parent
//! basic block of |pTarget|. This function assumes that |pInst| does not
//! already hav a parent basic block.
SPBE_API void 
spbe_instr_insert_after(SpbeInstruction* pInst, SpbeInstruction* pTarget);

//! Detach instruction |pInst| from its parent basic block, if it has one.
//! This does not destroy the instruction.
SPBE_API void 
spbe_instr_detach(SpbeInstruction* pInst);

//! Test if instruction |pInst| is considered trivially dead. An instruction
//! is considered trivially dead if it is non-volatile, non-branching, and
//! unused. Calls to functions or intrinsics without side effects may also be
//! considered trivially dead.
SPBE_API u32 
spbe_instr_is_trivially_dead(const SpbeInstruction* pInst);

//! Add the value |pValue| incoming from basic block |pPred| as an incoming
//! edge to the phi instruction |pInst|.
SPBE_API void 
spbe_instr_add_incoming(SpbeInstruction* pInst, SpbeValue* pValue, 
                        SpbeBasicBlock* pPred);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SPBE_INSTRUCTION_H_
