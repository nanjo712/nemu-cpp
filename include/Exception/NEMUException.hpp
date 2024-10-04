#ifndef NEMU_EXCEPTION_H_
#define NEMU_EXCEPTION_H_

#include <exception>

class invalid_instruction : public std::exception
{
    const char* what() const noexcept override { return "Invalid instruction"; }
};

class ebreak_exception : public std::exception
{
    const char* what() const noexcept override { return "Ebreak exception"; }
};

class program_halt : public std::exception
{
    const char* what() const noexcept override { return "Program halt"; }
};

class invalid_address : public std::exception
{
    const char* what() const noexcept override { return "Invalid address"; }
};

#endif  // NEMU_EXCEPTION_H_
