#ifndef NEMU_EXCEPTION_H_
#define NEMU_EXCEPTION_H_

#include <exception>

class invalid_instruction : public std::exception
{
    const char* what() const noexcept override { return "Invalid instruction"; }
};

#endif  // NEMU_EXCEPTION_H_
