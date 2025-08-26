#include "siir/argument.hpp"

using namespace stm;
using namespace stm::siir;

Argument::Argument(u32 num, const siir::Type* type, const std::string& name)
    : Value(type, name), m_argno(num) {}

BlockArgument::BlockArgument(u32 num, const Type* type, const std::string& name, 
                             BasicBlock* parent)
    : Argument(num, type, name), m_parent(parent) {}

BlockArgument* 
BlockArgument::create(u32 num, const Type* type, const std::string& name, 
                      BasicBlock* parent) {
    return new BlockArgument(num, type, name, parent);
}

FunctionArgument::FunctionArgument(u32 num, const Type* type, 
                                   const std::string& name, Function* parent)
    : Argument(num, type, name), m_parent(parent) {}

FunctionArgument* 
FunctionArgument::create(u32 num, const Type* type, const std::string& name,
                         Function* parent) {
    return new FunctionArgument(num, type, name, parent);
}
