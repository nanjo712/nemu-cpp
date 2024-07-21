#ifndef RISCV32_UTILS_H_
#define RISCV32_UTILS_H_

#include <array>

#include "Utils/Utils.h"

inline std::array<word_t, 5> img = {
    0x00000297,  // auipc t0,0
    0x00028823,  // sb  zero,16(t0)
    0x0102c503,  // lbu a0,16(t0)
    0x00100073,  // ebreak (used as nemu_trap)
    0xdeadbeef,  // some data
};

#endif  // RISCV32_UTILS_H_