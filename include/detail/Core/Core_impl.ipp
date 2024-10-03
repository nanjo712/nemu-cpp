#ifndef CORE_IMPL_IPP_
#define CORE_IMPL_IPP_

#include "detail/Core/Core_decl.hpp"

template <typename T>
Core<T>::Core()
{
}

template <typename T>
Core<T>::~Core()
{
}

template <typename T>
void Core<T>::execute_one_inst()
{
    single_instruction();
}

template <typename T>
void Core<T>::reset()
{
    static_cast<T*>(this)->reset_impl();
}

template <typename T>
void Core<T>::single_instruction()
{
    static_cast<T*>(this)->single_instruction_impl();
}

template <typename T>
auto Core<T>::debug_get_reg_val(int reg_num)
{
    return static_cast<T*>(this)->debug_get_reg_val_impl(reg_num);
}

template <typename T>
auto Core<T>::debug_get_pc()
{
    return static_cast<T*>(this)->pc;
}

template <typename T>
auto Core<T>::debug_get_reg_index(std::string_view reg_name)
{
    return static_cast<T*>(this)->debug_get_reg_index_impl(reg_name);
}

#endif  // CORE_IMPL_IPP_