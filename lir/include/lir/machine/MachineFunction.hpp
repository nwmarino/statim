//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_FUNCTION_H_
#define LIR_MACHINE_FUNCTION_H_

#include "lir/machine/MachineData.hpp"
#include "lir/machine/MachineLabel.hpp"

#include <cstdint>
#include <vector>

namespace lir {

class ConstantPool;
class StackFrame;
class MachineFunction;
class MachineObject;

/// A local that lives within the stack frame of a function.
class MachineLocal final {
    StackFrame *m_parent;

    /// The byte offset of this local within its parent stack frame.
    int32_t m_offset;

    /// The size reserved for this local and the alignment of it, in bytes.
    uint32_t m_size, m_align;

public:
    MachineLocal(StackFrame *parent, int32_t offset, uint32_t size, 
                 uint32_t align);

    void set_parent(StackFrame *frame) { m_parent = frame; }
    const StackFrame *get_parent() const { return m_parent; }
    StackFrame *get_parent() { return m_parent; }

    /// Test if this local has a parent stack frame.
    bool has_parent() const { return m_parent != nullptr; }

    /// Returns the machine function which this local ultimately belongs to.
    const MachineFunction *get_function() const;
    MachineFunction *get_function() {
        return const_cast<MachineFunction*>(
            static_cast<const MachineLocal*>(this)->get_function());
    }

    void set_offset(int32_t offset) { m_offset = offset; }
    int32_t get_offset() const { return m_offset; }

    void set_size(uint32_t size) { m_size = size; }
    uint32_t get_size() const { return m_size; }

    void set_align(uint32_t align) { m_align = align; }
    uint32_t get_align() const { return m_align; }
};

class ConstantPool final {
public:
    using Constants = std::vector<MachineData*>;

private:
    MachineFunction *m_parent;

    /// The constants of this pool.
    Constants m_constants = {};

public:
    ConstantPool() = default;

    ~ConstantPool();

    ConstantPool(const ConstantPool&) = delete;
    void operator=(const ConstantPool&) = delete;

    ConstantPool(ConstantPool&&) noexcept = delete;
    void operator=(ConstantPool&&) noexcept = delete;

    void set_parent(MachineFunction *func) { m_parent = func; }
    const MachineFunction *get_parent() const { return m_parent; }
    MachineFunction *get_parent() { return m_parent; }
    
    /// Test if this constant pool belongs to a function.
    bool has_parent() const { return m_parent != nullptr; }

    const Constants &get_constants() const { return m_constants; }
    Constants &get_constants() { return m_constants; }

    /// Returns the number of constants in this pool.
    uint32_t num_constants() const { return m_constants.size(); }

    /// Test if this constant pool is empty i.e. contains no data.
    bool empty() const { return m_constants.empty(); }
};

/// Representation of a stack frame for a machine function.
class StackFrame final {
public:
    using Locals = std::vector<MachineLocal*>;

private:
    MachineFunction *m_parent;

    /// The locals of this stack frame.
    Locals m_locals = {};

public:
    StackFrame() = default;

    ~StackFrame();

    StackFrame(const StackFrame&) = delete;
    void operator=(const StackFrame&) = delete;

    StackFrame(StackFrame&&) noexcept = delete;
    void operator=(StackFrame&&) noexcept = delete;

    void set_parent(MachineFunction *func) { m_parent = func; }
    const MachineFunction *get_parent() const { return m_parent; }
    MachineFunction *get_parent() { return m_parent; }
    
    /// Test if this stack frame belongs to a function.
    bool has_parent() const { return m_parent != nullptr; }

    const Locals &get_locals() const { return m_locals; }
    Locals &get_locals() { return m_locals; }

    /// Returns the number of locals in this stack frame.
    uint32_t num_locals() const { return m_locals.size(); }

    /// Test if this stack frame is empty i.e. contains no locals.
    bool empty() const { return m_locals.empty(); }
};

class MachineFunction final {
public:
    using Labels = std::vector<MachineLabel*>;

private:
    MachineObject *m_parent;
    std::string m_name;
    ConstantPool m_pool = {};
    StackFrame m_frame = {};
    Labels m_labels = {};

public:
    MachineFunction(MachineObject *parent, const std::string &name);

    ~MachineFunction();

    MachineFunction(const MachineFunction&) = delete;
    void operator=(const MachineFunction&) = delete;

    MachineFunction(MachineFunction&&) noexcept = delete;
    void operator=(MachineFunction&&) noexcept = delete;

    void set_parent(MachineObject *obj) { m_parent = obj; }
    const MachineObject *get_parent() const { return m_parent; }
    MachineObject *get_parent() { return m_parent; }

    /// Test if this function belongs to a parent object.
    bool has_parent() const { return m_parent != nullptr; }

    const std::string &get_name() const { return m_name; }

    const ConstantPool &get_pool() const { return m_pool; }
    ConstantPool &get_pool() { return m_pool; }

    const StackFrame &get_stack_frame() const { return m_frame; }
    StackFrame &get_stack_frame() { return m_frame; }

    const Labels &labels() const { return m_labels; }
    Labels &labels() { return m_labels; }

    /// Returns the number of labels in this function.
    uint32_t num_labels() const { return m_labels.size(); }

    /// Returns the |i|-th label in this function.
    const MachineLabel *get_label(uint32_t i) const {
        assert(i < num_labels() && "index out of bounds!");
        return m_labels[i];
    }

    MachineLabel *get_label(uint32_t i) {
        assert(i < num_labels() && "index out of bounds!");
        return m_labels[i];
    }

    /// Add the given |label| to the back of this function.
    void add(MachineLabel *label);

    /// Remove the given |label| from this function, if it belongs.
    void remove(MachineLabel *label);

    /// Update the positions of labels and ops in this function.
    /// This should be used after passes which mutate the IR state.
    void update_positions();

    /// Returns the position of the given |label| in this function.
    /// Fails if |label| does not belong to this function.
    uint32_t get_position(const MachineLabel *label) const;
};

} // namespace lir

#endif // LIR_MACHINE_FUNCTION_H_
