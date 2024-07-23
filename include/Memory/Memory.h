#ifndef MEMORY_H_
#define MEMORY_H_

#include <cstdint>
#include <type_traits>

#include "Utils/Utils.h"

class Memory
{
   public:
    using paddr_t =
        std::conditional_t<MEMORY_SIZE + MEMORY_BASE <= 0x100000000ul, uint32_t,
                           uint64_t>;
    word_t read(paddr_t addr, int len);
    void write(paddr_t addr, word_t data, int len);
    static Memory& getMemory();

    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    Memory(Memory&&) = delete;
    Memory& operator=(Memory&&) = delete;

    ~Memory();

   private:
    const paddr_t lower_bound = MEMORY_BASE;
    const paddr_t upper_bound = MEMORY_BASE + MEMORY_SIZE;
    constexpr bool inRange(paddr_t addr) const;

    Memory();

    uint8_t physicalMemory[MEMORY_SIZE];
    uint8_t* getHostMemAddr(paddr_t paddr);
};

#endif  // MEMORY_H_