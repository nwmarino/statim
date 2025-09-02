#ifndef STATIM_TRANSLATION_UNIT_HPP_
#define STATIM_TRANSLATION_UNIT_HPP_

#include "tree/root.hpp"
#include "types/input_file.hpp"

#include <cassert>
#include <memory>

namespace stm {

class TranslationUnit final {
    InputFile& file;
    
    std::unique_ptr<Root> pRoot = nullptr;

public:
    TranslationUnit(InputFile& file) : file(file) {};

    InputFile& get_file() { return file; }

    Root& get_root() { 
        assert(pRoot && "root not created yet");
        return *pRoot; 
    }

    const Root& get_root() const { 
        assert(pRoot && "root not created yet");
        return *pRoot; 
    }

    void set_root(std::unique_ptr<Root> root) {
        pRoot = std::move(root);
    }
};

} // namespace stm

#endif // STATIM_TRANSLATION_UNIT_HPP_
