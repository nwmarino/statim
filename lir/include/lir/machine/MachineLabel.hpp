//
//  Copyright (c) 2025-2026 Nicholas Marino
//  All rights reserved.
//

#ifndef LIR_MACHINE_LABEL_H_
#define LIR_MACHINE_LABEL_H_

#include "lir/machine/MachineOp.hpp"

namespace lir {

class MachineLabel final {
    MachineFunction *m_parent;
    MachineOp *m_head = nullptr;
    MachineOp *m_tail = nullptr;

public:
    MachineLabel(MachineFunction *parent = nullptr);

    void set_parent(MachineFunction *func) { m_parent = func; }
    const MachineFunction *get_parent() const { return m_parent; }
    MachineFunction *get_parent() { return m_parent; }

    /// Test if this label belongs to a function.
    bool has_parent() const { return m_parent != nullptr; }

    void set_head(MachineOp *op) { m_head = op; }
    const MachineOp *get_head() const { return m_head; }
    MachineOp *get_head() { return m_head; }

    void set_tail(MachineOp *op) { m_tail = op; }
    const MachineOp *get_tail() const { return m_tail; }
    MachineOp *get_tail() { return m_tail; }

    /// Returns the size of this label by the number of ops in it.
    uint32_t size() const {
        uint32_t res = 0;
        const MachineOp *curr = m_head;
        while (curr) {
            res++;
            curr = curr->get_next();
        }

        return res;
    }

    /// Test if this label is empty i.e. contains no ops.
    bool empty() const { return m_head == nullptr; }

    /// Prepend the given |op| to the front of this label.
    void prepend(MachineOp *op);

    /// Append the given |op| to the back of this label.
    void append(MachineOp *op);

    /// Remove the given |op| from this label.
    void remove(MachineOp *op);

    /// Returns the position of this label relative to others in its parent.
    /// Fails if this label does not belong to a function.
    uint32_t position() const;
};

} // namespace lir

#endif // LIR_MACHINE_LABEL_H_
