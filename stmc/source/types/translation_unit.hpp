#ifndef STATIM_TRANSLATION_UNIT_HPP_
#define STATIM_TRANSLATION_UNIT_HPP_

#include "siir/cfg.hpp"
#include "tree/root.hpp"
#include "types/input_file.hpp"

#include <cassert>
#include <memory>

namespace stm {

class TranslationUnit final {
    InputFile& m_file;
    std::unique_ptr<Root> m_root = nullptr;
    std::unique_ptr<siir::CFG> m_graph = nullptr; 

public:
    TranslationUnit(InputFile& file) : m_file(file) {};

    const InputFile& get_file() const { return m_file; }
    InputFile& get_file() { return m_file; }

    const Root& get_root() const { return *m_root; }
    Root& get_root() { return *m_root; }

    void set_root(std::unique_ptr<Root> root) {
        m_root = std::move(root);
    }

    const siir::CFG& get_graph() const { return *m_graph; }
    siir::CFG& get_graph() { return *m_graph; }

    void set_graph(std::unique_ptr<siir::CFG> graph) {
        m_graph = std::move(graph);
    }
};

} // namespace stm

#endif // STATIM_TRANSLATION_UNIT_HPP_
