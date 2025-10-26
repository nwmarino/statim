#ifndef SPBE_VALUE_H_
#define SPBE_VALUE_H_

#include "common.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct SpbeUser SpbeUser;
typedef struct SpbeValue SpbeValue;

typedef struct SpbeUse {
    SpbeValue*  value;
    SpbeUser*   user;
    u32         index;
} SpbeUse;

typedef enum SpbeValueKind {
    SPBE_VALUE_KIND_ARGUMENT,
    SPBE_VALUE_KIND_FUNCTION,
    SPBE_VALUE_KIND_INLINE_ASSEMBLY,
    SPBE_VALUE_KIND_LOCAL,
    SPBE_VALUE_KIND_USER,
} SpbeValueKind;

typedef struct SpbeValue {
    SpbeValueKind   kind;
    u32             num_uses;
    u32             uses_cap;
    SpbeUse**       uses;
    SpbeType*       type;
} SpbeValue;

typedef enum SpbeUserKind {
    SPBE_USER_KIND_CONSTANT,
    SPBE_USER_KIND_INSTRUCTION,
} SpbeUserKind;

typedef struct SpbeUser {
    SpbeValue       base;
    SpbeUserKind    kind;
    u32             num_operands;
    u32             operands_cap;
    SpbeUse**       operands;
} SpbeUser;

//! Adds the use edge |pUse| to the value |pValue|.
SPBE_API void spbe_value_add_use(SpbeValue* pValue, SpbeUse* pUse);

//! Returns the use edge of value |pValue| at index |i|, if it exists.
SPBE_API SpbeUse* spbe_value_use_at(SpbeValue* pValue, u32 i);

//! Replace all the uses of value |pValue| with the value |pNew|.
SPBE_API void spbe_value_replace_all_uses_with(SpbeValue* pValue, 
                                               SpbeValue* pNew);


//! Creates a new use edge between value |pValue| and a user |pUser|.
SPBE_API SpbeUse* spbe_create_use(SpbeValue* pValue, SpbeUser* pUser);

//! Destroy the use edge |pUse|.
SPBE_API void spbe_destroy_use(SpbeUse* pUse);


//! Add a new operand value |pOperand| to the user |pUser|.
SPBE_API void spbe_user_add_operand(SpbeUser* pUser, SpbeValue* pOperand);

//! Set the operand at index |i| of user |pUser| to the value |pOperand|.
SPBE_API void spbe_user_set_operand(SpbeUser* pUser, SpbeValue* pOperand, u32 i);

//! Clear the operand at index |i| of user |pUser|.
SPBE_API void spbe_user_clear_operand(SpbeUser* pUser, u32 i);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SPBE_VALUE_H_
