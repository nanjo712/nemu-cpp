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

#endif  // CORE_IMPL_IPP_