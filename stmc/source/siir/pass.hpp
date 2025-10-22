#ifndef STATIM_SIIR_PASS_HPP_
#define STATIM_SIIR_PASS_HPP_

#include "siir/cfg.hpp"

namespace stm {
namespace siir {
     
class Pass {
protected:
    CFG& m_cfg;

public:
    Pass(CFG& cfg) : m_cfg(cfg) {}

    Pass(const Pass&) = delete;
    Pass& operator = (const Pass&) = delete;

    virtual ~Pass() = default;

    virtual void run() = 0;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_PASS_HPP_
