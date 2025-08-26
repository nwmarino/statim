#ifndef STATIM_SIIR_GLOBAL_HPP_
#define STATIM_SIIR_GLOBAL_HPP_

#include "siir/constant.hpp"

namespace stm {

namespace siir {

class CFG;

class Global final : public Constant {
public:
    enum LinkageTypes : u8 {
        Internal, External
    };

private:
    CFG* m_parent;
    Constant* m_init;
    LinkageTypes m_linkage;
    bool m_readonly;
    
    Global(const Type* type, LinkageTypes linkage, bool read_only, 
           Constant* init, CFG* parent, const std::string& name);

public:
    static Global* create(const Type* type, LinkageTypes linkage, 
                          bool read_only, CFG* parent, Constant* init = nullptr, 
                          const std::string& name = "");

    LinkageTypes get_linkage() const { return m_linkage; }
    void set_linkage(LinkageTypes linkage) { m_linkage = linkage; }
    
    bool is_read_only() const { return m_readonly; }
    void set_read_only(bool value = true) { m_readonly = value; }

    const CFG* get_parent() const { return m_parent; }
    CFG* get_parent() { return m_parent; }
    void set_parent(CFG* parent) { m_parent = parent; }
    void clear_parent() { m_parent = nullptr; }

    Constant* get_initializer() const { return m_init; }
    void set_initializer(Constant* constant) { m_init = constant; }
    bool has_initializer() const { return m_init != nullptr; }

    void print(std::ostream& os) const override;
};

} // namespace siir
    
} // namespace stm

#endif // STATIM_SIIR_GLOBAL_HPP_
