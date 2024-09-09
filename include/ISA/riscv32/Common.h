#ifndef ISA_RISCV32_COMMON_H_
#define ISA_RISCV32_COMMON_H_

#include <cstdint>

namespace detail
{
    namespace riscv32
    {
        using word_t = uint32_t;
        using sword_t = int32_t;

        constexpr word_t PC_RESET = 0x80000000;
    }  // namespace riscv32
}  // namespace detail

#endif  // ISA_RISCV32_COMMON_H_