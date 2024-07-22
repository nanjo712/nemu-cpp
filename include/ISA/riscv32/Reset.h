#ifndef RISCV32_UTILS_H_
#define RISCV32_UTILS_H_

#include <array>

#include "Utils/Utils.h"

class Register;
class Memory;

class Reset_Handler
{
   public:
    Reset_Handler(Register& reg);
    ~Reset_Handler();
    const std::array<word_t, 5>& get_img();
    void reset();
    word_t get_reset_vector() const { return reset_vector; }

   private:
    const word_t reset_vector = MEMORY_BASE + RESET_PC_OFFSET;
    const std::array<word_t, 5> img = {
        0x00000297,  // auipc t0,0
        0x00028823,  // sb  zero,16(t0)
        0x0102c503,  // lbu a0,16(t0)
        0x00100073,  // ebreak (used as nemu_trap)
        0xdeadbeef,  // some data
    };  // built-in imag
    Register& reg;
};

#endif  // RISCV32_UTILS_H_