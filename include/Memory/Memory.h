#ifndef MEMORY_H_
#define MEMORY_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "Utils/Utils.h"

class Memory
{
   public:
    using paddr_t = decltype(MEMORY_BASE + MEMORY_SIZE);
    using vaddr_t = paddr_t;

    word_t inst_fetch(vaddr_t addr, int len);
    word_t vread(vaddr_t addr, int len);
    void vwrite(vaddr_t addr, word_t data, int len);

    word_t debug_vread(vaddr_t addr, int len);
    void load_image(std::vector<uint8_t>& image);

    Memory();
    ~Memory();

   private:
    static constexpr paddr_t lower_bound = MEMORY_BASE;
    static constexpr paddr_t upper_bound = MEMORY_BASE + MEMORY_SIZE;
    constexpr bool in_range(paddr_t addr) const;

    std::unique_ptr<std::array<uint8_t, MEMORY_SIZE>> physicalMemory;
    uint8_t* get_host_memory_addr(paddr_t paddr);

    word_t pread(paddr_t addr, int len);
    void pwrite(paddr_t addr, word_t data, int len);
};

#endif  // MEMORY_H_