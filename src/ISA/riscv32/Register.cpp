#include "ISA/riscv32/Register.h"

#include <cassert>

const std::string Register::regNames[32] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

Register::Register()
{
    for (int i = 0; i < 32; i++)
    {
        registers[i] = 0;
    }
    pc = 0x80000000;
}

Register::~Register() {}

int Register::getRegIndex(const std::string_view regName)
{
    for (int i = 0; i < 32; i++)
    {
        if (regNames[i] == regName)
        {
            return i;
        }
    }
    assert(false);
    return -1;
}

void Register::write(int reg, word_t data)
{
    if (reg == 0)
    {
        return;
    }
    registers[reg] = data;
}

word_t Register::read(int reg) { return registers[reg]; }

void Register::setPC(word_t data) { pc = data; }

word_t Register::getPC() { return pc; }