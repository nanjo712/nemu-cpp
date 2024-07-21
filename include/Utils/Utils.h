#ifndef UTILS_H_
#define UTILS_H_

#include <cstdint>

#ifdef ISA_64
using word_t = uint64_t;
using sword_t = int64_t;
#define FMT_WORD "0x%016lx"
#else
using word_t = uint32_t;
using sword_t = int32_t;
#define FMT_WORD "0x%08x"
#endif

template <typename T>
constexpr T extract_bits(T data, int start, int end)
{
    return (data >> start) & ((1 << (end - start + 1)) - 1);
}

template <typename T>
constexpr T sign_extend(T data, int size)
{
    T sign_bit = (data >> (size - 1)) & 1;
    if (sign_bit)
    {
        data |= ~((1 << size) - 1);
    }
    return data;
}

#endif  // UTILS_H_