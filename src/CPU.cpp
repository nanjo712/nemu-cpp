#include "CPU/CPU.h"

#include "ISA/ISA_Wrapper.h"
#include "Memory/Memory.h"

CPU::CPU(ISA_Wrapper& isa) : isa(isa) {};

CPU::~CPU() {};

void CPU::execute(uint64_t n)
{
    for (uint64_t i = 0; i < n; i++)
    {
        isa.execute_one_inst();
    }
}
