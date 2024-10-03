#ifndef MEMORY_H_
#define MEMORY_H_

#include <cstdint>
#include <memory>

#include "Utils/Utils.h"

class Memory
{
   public:
    using paddr_t = decltype(MEMORY_BASE + MEMORY_SIZE);
    word_t read(paddr_t addr, int len);
    void write(paddr_t addr, word_t data, int len);

    Memory();
    ~Memory();

   private:
    static constexpr paddr_t lower_bound = MEMORY_BASE;
    static constexpr paddr_t upper_bound = MEMORY_BASE + MEMORY_SIZE;
    constexpr bool inRange(paddr_t addr) const;

    std::unique_ptr<std::array<uint8_t, MEMORY_SIZE>> physicalMemory;
    uint8_t* getHostMemAddr(paddr_t paddr);
};

#endif  // MEMORY_H_