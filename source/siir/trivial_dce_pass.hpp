#ifndef STATIM_SIIR_TRIVIAL_DCE_PASS_HPP_
#define STATIM_SIIR_TRIVIAL_DCE_PASS_HPP_

#include "siir/pass.hpp"

namespace stm {
namespace siir {

class TrivialDCEPass final : public Pass {
public:
    TrivialDCEPass(CFG& cfg) : Pass(cfg) {}

    void run() override;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_TRIVIAL_DCE_PASS_HPP_
