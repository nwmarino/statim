#ifndef STATIM_MACHINE_TARGET_HPP_
#define STATIM_MACHINE_TARGET_HPP_

#include "types.hpp"

namespace stm {

class Target final {
public:
    enum Arch : u8 {
        amd64,
    };

    enum OS : u8 {
        Linux,
    };

    enum ABI : u8 {
        SystemV,
    };

private:
    Arch m_arch;
    OS m_os;
    ABI m_abi;

public:
    Target(Arch arch, OS os, ABI abi) : m_arch(arch), m_os(os), m_abi(abi) {}

    Arch arch() const { return m_arch; }

    OS os() const { return m_os; }

    ABI abi() const { return m_abi; }
};

} // namespace stm

#endif // STATIM_MACHINE_TARGET_HPP_
