#include "Memory/Memory.h"

#include <spdlog/spdlog.h>

#include <cassert>
// #include <random>

Memory::Memory()
    : physicalMemory(std::make_unique<std::array<uint8_t, MEMORY_SIZE>>())
{
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<> dis(0, 255);
    // for (int i = 0; i < MEMORY_SIZE; i++)
    // {
    //     physicalMemory->at(i) = dis(gen);
    // }
    // spdlog::info("Memory initialized with random data.");
    spdlog::info("Memory size: {} bytes", MEMORY_SIZE);
    spdlog::info("Memory base: 0x{:08x}", MEMORY_BASE);
    spdlog::info("Memory upper bound: 0x{:08x}", upper_bound);
}

constexpr bool Memory::in_range(paddr_t addr) const
{
    return addr >= lower_bound && addr < upper_bound;
}

Memory::~Memory() { /*spdlog::info("Memory destroyed.");*/ }

uint8_t* Memory::get_host_memory_addr(paddr_t paddr)
{
    if (!in_range(paddr))
    {
        spdlog::error("Physical address 0x{:08x} out of range.", paddr);
        assert(false);
    }
    return &(physicalMemory->at(0)) + (paddr - lower_bound);
}

word_t Memory::read(paddr_t addr, int len)
{
    if (len != 1 && len != 2 && len != 4 && len != 8)
    {
        spdlog::error("Invalid read length: {}", len);
        assert(false);
    }

    word_t data = 0;
    auto hostMemAddr = get_host_memory_addr(addr);
    for (int i = 0; i < len; i++)
    {
        data |= hostMemAddr[i] << (i * 8);
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

    auto hostMemAddr = get_host_memory_addr(addr);

    for (int i = 0; i < len; i++)
    {
        hostMemAddr[i] = (data >> (i * 8)) & 0xff;
    }
}

void Memory::load_image(std::vector<uint8_t>& image)
{
    if (image.size() > MEMORY_SIZE)
    {
        spdlog::error("Image size exceeds memory size.");
        assert(false);
    }

    for (size_t i = 0; i < image.size(); i++)
    {
        physicalMemory->at(i) = image[i];
    }
}
