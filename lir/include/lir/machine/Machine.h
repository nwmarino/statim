//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_H_
#define LIR_MACHINE_H_

#include "lir/graph/Type.h"
#include "lir/machine/Register.h"

namespace lir {

/// Instances of the Machine class help describe the specifics of a compiler 
/// target, and in particular things relating to type size, alignment and ABI 
/// nature.
class Machine final {
public:
    /// The different recognized operating systems.
    enum OS : uint8_t {
        Linux, Windows,
    };

    /// Type information for system pointers.
    struct Pointer final {
        uint8_t size, align;
    };

private:
    OS m_os;
    bool m_little_endian;
    Pointer m_pointer;

public:
    explicit Machine(OS os);

    inline bool is_little_endian() const { return m_little_endian; }
    inline bool is_big_endian() const { return !m_little_endian; }

    /// Returns the size of a pointer for this machine, in bits.
    uint32_t get_pointer_size() const { return m_pointer.size; }

    /// Returns the natural alignment of a pointer for this machine, in bits.
    uint32_t get_pointer_align() const { return m_pointer.align; }

    /// Returns the size of the given |type|, in bits.
    uint32_t get_type_size(const Type *type) const;

    /// Returns the natural alignment of the given |type|, in bits.
    uint32_t get_type_align(const Type *type) const;

    /// Test if the given |type| is considered scalar for this machine.
    bool is_scalar(const Type *type) const;

    /// Returns the byte offset for the |i|-th element of the given |type|.
    uint32_t get_element_offset(const ArrayType *type, uint32_t i) const;

    /// Returns the byte offset for the |i|-th pointee of the given |type|.
    uint32_t get_pointee_offset(const PointerType *type, uint32_t i) const;

    /// Returns the byte offset for the |i|-th field of the given |type|.
    uint32_t get_field_offset(const StructType *type, uint32_t i) const;

    bool isCallerSaved(uint32_t reg) const;
};

} // namespace lir

#endif // LIR_MACHINE_H_
