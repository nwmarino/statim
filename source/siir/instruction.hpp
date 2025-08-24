#ifndef STATIM_SIIR_INSTRUCTION_HPP_
#define STATIM_SIIR_INSTRUCTION_HPP_

#include "siir/user.hpp"

namespace stm {

class Instruction : public User {


public:
    Instruction* prev() const;
    Instruction* next() const;
};

class StoreInst final : public Instruction {

};

class LoadInst final : public Instruction {

};

class BrifInst final : public Instruction {

};

class JmpInst final : public Instruction {

};

class RetInst final : public Instruction {

};

class CallInst final : public Instruction {

};

class CmpInst final : public Instruction {

};

class BinopInst final : public Instruction {
    
};

class UnopInst final : public Instruction {

};

} // namespace stm

#endif // STATIM_SIIR_INSTRUCTION_HPP_
