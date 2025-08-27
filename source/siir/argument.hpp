#ifndef STATIM_SIIR_ARGUMENT_HPP_
#define STATIM_SIIR_ARGUMENT_HPP_

#include "siir/value.hpp"
#include "types/types.hpp"

namespace stm {

namespace siir {

class BasicBlock;
class Function;

/// Base class for all argument values.
class Argument : public Value {
protected:
    u32 m_argno;

    Argument(u32 num, const Type* type, const std::string& name);

public:
    u32 get_arg_number() const { return m_argno; }
    void set_arg_number(u32 num) { m_argno = num; }
};

/// Represents an argument to a BasicBlock.
class BlockArgument final : public Argument {
    BasicBlock* m_parent;

    BlockArgument(u32 num, const Type* type, const std::string& name, 
                  BasicBlock* parent);

public:
    static BlockArgument* create(u32 num, const Type* type, 
                                 const std::string& name = "", 
                                 BasicBlock* parent = nullptr);

    const BasicBlock* get_parent() const { return m_parent; }
    BasicBlock* get_parent() { return m_parent; }
    void set_parent(BasicBlock* parent) { m_parent = parent; }
    void clear_parent() { m_parent = nullptr; }

    void print(std::ostream& os) const override;
};

/// Represents an argument to a Function.
class FunctionArgument final : public Argument {
    Function* m_parent;

    FunctionArgument(u32 num, const Type* type, const std::string& name,
                     Function* parent);

public:
    static FunctionArgument* create(u32 num, const Type* type,
                                    const std::string& name = "",
                                    Function* parent = nullptr);

    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }
    void set_parent(Function* parent) { m_parent = parent; }
    void clear_parent() { m_parent = nullptr; }

    void print(std::ostream& os) const override;
};

} // namespace siir

} // namespace stm

#endif // STATIM_SIIR_ARGUMENT_HPP_
