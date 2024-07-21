#include "Memory/Memory.h"

#include <spdlog/spdlog.h>

#include <random>

Memory::Memory()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        physicalMemory[i] = dis(gen);
    }
    spdlog::info("Memory initialized with random data.");
    spdlog::info("Memory size: {} bytes", MEMORY_SIZE);
    spdlog::info("Memory base: 0x{:08x}", MEMORY_BASE);
    spdlog::info("Memory upper bound: 0x{:08x}", upper_bound);
}

constexpr bool Memory::inRange(paddr_t addr) const
{
    return addr >= lower_bound && addr < upper_bound;
}

Memory::~Memory() { spdlog::info("Memory destroyed."); }

uint8_t *Memory::getHostMemAddr(paddr_t paddr)
{
    if (!inRange(paddr))
    {
        spdlog::error("Physical address 0x{:08x} out of range.", paddr);
        assert(false);
    }
    return physicalMemory + (paddr - lower_bound);
}

word_t Memory::read(paddr_t addr, int len)
{
    if (len != 1 && len != 2 && len != 4 && len != 8)
    {
        spdlog::error("Invalid read length: {}", len);
        assert(false);
    }
    word_t data = 0;
    for (int i = 0; i < len; i++)
    {
        data |= getHostMemAddr(addr)[i] << (i * 8);
    }
    return data;
}

void Memory::write(paddr_t addr, word_t data, int len)
{
    if (len != 1 && len != 2 && len != 4 && len != 8)
    {
        spdlog::error("Invalid write length: {}", len);
        assert(false);
    }
    for (int i = 0; i < len; i++)
    {
        getHostMemAddr(addr)[i] = (data >> (i * 8)) & 0xff;
    }
}
