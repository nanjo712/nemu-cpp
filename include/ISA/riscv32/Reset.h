#ifndef RISCV32_UTILS_H_
#define RISCV32_UTILS_H_

#include <array>
#include <string>
#include <vector>

#include "Utils/Utils.h"

class Register;
class Memory;

class Reset_Handler
{
   public:
    Reset_Handler(Register& reg);
    ~Reset_Handler();
    void set_img(const std::string& img_file);
    const std::vector<word_t>& get_img();
    void reset();
    word_t get_reset_vector() const { return reset_vector; }

   private:
    const word_t reset_vector = MEMORY_BASE + RESET_PC_OFFSET;
    const std::array<word_t, 5> built_in_img = {
        0x00000297,  // auipc t0,0
        0x00028823,  // sb  zero,16(t0)
        0x0102c503,  // lbu a0,16(t0)
        0x00100073,  // ebreak (used as nemu_trap)
        0xdeadbeef,  // some data
    };  // built-in image
    Register& reg;
    std::vector<word_t> img;
};

#endif  // RISCV32_UTILS_H_