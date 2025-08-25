#ifndef STATIM_SIIR_USE_HPP_
#define STATIM_SIIR_USE_HPP_

namespace stm {
    
class Value;
class User;

class Use final {
    Value* m_value;
    User* m_user;
};

} // namespace stm

#endif // STATIM_SIIR_USE_HPP_
