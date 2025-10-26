#ifndef SPBE_BASICBLOCK_H_
#define SPBE_BASICBLOCK_H_

#include "common.h"
#include "instruction.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct SpbeFunction SpbeFunction;

typedef struct SpbeBasicBlock {
    SpbeFunction*       pParent;
    SpbeBasicBlock*     pPrev;
    SpbeBasicBlock*     pNext;
    SpbeInstruction*    pFront;
    SpbeInstruction*    pBack;
    u32                 num_preds;
    SpbeBasicBlock**    ppPreds;
    u32                 num_succs;
    SpbeBasicBlock**    ppSuccs;
} SpbeBasicBlock;

//! Create a new basic block with parent function |pParent|.
SPBE_API SpbeBasicBlock* 
spbe_basicblock_create(SpbeFunction* pParent);

//! Destroy basic block |pBlock| and detach it from any parent function.
SPBE_API void 
spbe_basicblock_destroy(SpbeBasicBlock* pBlock);

//! Append the basic block |pBlock| to function |pFunc|. This function assumes
//! that |pBlock| does not have any parent function already.
SPBE_API void 
spbe_basicblock_append_to(SpbeBasicBlock* pBlock, SpbeFunction* pFunc);

SPBE_API void
spbe_basicblock_insert_before(SpbeBasicBlock* pBlock, SpbeBasicBlock* pTarget);

SPBE_API void
spbe_basicblock_insert_after(SpbeBasicBlock* pBlock, SpbeBasicBlock* pTarget);

SPBE_API void
spbe_basicblock_remove_instr(SpbeBasicBlock* pBlock, SpbeInstruction* pInst);

SPBE_API u32
spbe_basicblock_is_entry(SpbeBasicBlock* pBlock);

SPBE_API void
spbe_basicblock_detach(SpbeBasicBlock* pBlock);

SPBE_API void
spbe_basicblock_push_front(SpbeBasicBlock* pBlock, SpbeInstruction* pInst);

SPBE_API void
spbe_basicblock_push_back(SpbeBasicBlock* pBlock, SpbeInstruction* pInst);

SPBE_API void
spbe_basicblock_insert(SpbeBasicBlock* pBlock, SpbeInstruction* pInst, u32 i);

SPBE_API u32
spbe_basicblock_is_empty(SpbeBasicBlock* pBlock);

SPBE_API u32
spbe_basicblock_get_position(SpbeBasicBlock* pBlock);

SPBE_API u32
spbe_basicblock_get_size(SpbeBasicBlock* pBlock);

SPBE_API u32
spbe_basicblock_terminates(SpbeBasicBlock* pBlock);

SPBE_API SpbeInstruction*
spbe_basicblock_terminator(SpbeBasicBlock* pBlock);

SPBE_API u32
spbe_basicblock_terminators(SpbeBasicBlock* pBlock);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SPBE_BASICBLOCK_H_
