#ifndef SPBE_TARGET_H_
#define SPBE_TARGET_H_

#include "common.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum SpbeArch {
    SPBE_TARGET_ARCHITECTURE_X86_64,
    SPBE_TARGET_ARCHITECTURE_ARM_64,
} SpbeArch;

typedef enum SpbeOpersys {
    SPBE_TARGET_OPERSYS_LINUX,
    SPBE_TARGET_OPERSYS_WINDOWS,
} SpbeOpersys;

typedef enum SpbeABI {
    SPBE_TARGET_ABI_SYSTEM_V,
} SpbeABI;

typedef struct SpbeTarget {
    SpbeABI abi;
    SpbeArch arch;
    SpbeOpersys opersys;
} SpbeTarget;

//! Returns the type size for |pType| in bytes for the target |pTarget|. 
SPBE_API u32 spbe_get_type_size(const SpbeTarget* pTarget, 
                                const SpbeType* pType);

//! Returns the naturally desirable type alignment size for |pType| in bytes
//! for the target |pTarget|.
SPBE_API u32 spbe_get_type_align(const SpbeTarget* pTarget, 
                                 const SpbeType* pType);

//! Returns true if the target architecture of |pTarget| is little endian.
SPBE_API u32 spbe_is_little_endian(const SpbeTarget* pTarget);

//! Returns true if the target architecture of |pTarget| is big endian.
SPBE_API u32 spbe_is_big_endian(const SpbeTarget* pTarget);

//! Returns the pointer size in bytes for |pTarget|.
SPBE_API u32 spbe_get_pointer_size(const SpbeTarget* pTarget);

//! Returns the natural pointer alignment size in bytes for |pTarget|.
SPBE_API u32 spbe_get_pointer_align(const SpbeTarget* pTarget);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SPBE_TARGET_H_
