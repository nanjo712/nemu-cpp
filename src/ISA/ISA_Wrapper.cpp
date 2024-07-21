#include "ISA/ISA_Wrapper.h"

#include <iostream>

ISA_Wrapper::ISA_Wrapper(Memory &mem) : mem(mem), reg(), inst(reg, mem) {};

ISA_Wrapper::~ISA_Wrapper() {};

void ISA_Wrapper::execute_one_inst()
{
    word_t pc = reg.getPC();
    word_t inst = mem.read(pc, 4);
    this->inst.execute(inst);
};

void ISA_Wrapper::display_reg()
{
    for (int i = 0; i < 32; i++)
    {
        std::cout << Register::regNames[i] << ": " << reg.read(i) << std::endl;
    }
    std::cout << "pc: " << reg.getPC() << std::endl;
};
