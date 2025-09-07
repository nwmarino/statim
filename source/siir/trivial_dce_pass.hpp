#ifndef STATIM_SIIR_TRIVIAL_DCE_PASS_HPP_
#define STATIM_SIIR_TRIVIAL_DCE_PASS_HPP_

#include "siir/instruction.hpp"
#include "siir/pass.hpp"

#include <vector>

namespace stm {
namespace siir {

class TrivialDCEPass final : public Pass {
    void process(Function* fn);

    std::vector<Instruction*> m_to_remove = {};

public:
    TrivialDCEPass(CFG& cfg) : Pass(cfg) {}

    void run() override;
};

} // namespace siir
} // namespace stm

#endif // STATIM_SIIR_TRIVIAL_DCE_PASS_HPP_
