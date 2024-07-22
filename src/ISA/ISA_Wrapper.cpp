#include "ISA/ISA_Wrapper.h"

#include <iostream>

#include "ISA/riscv32/Reset.h"

ISA_Wrapper::ISA_Wrapper(Memory& mem)
    : mem(mem), reg(), executor(reg, mem), reset_handler(reg)
{
    reset();
    load_img();
};

ISA_Wrapper::~ISA_Wrapper() {};

void ISA_Wrapper::execute_one_inst()
{
    word_t pc = reg.getPC();
    word_t inst = mem.read(pc, 4);
    executor.execute(inst);
};

void ISA_Wrapper::display_reg()
{
    for (int i = 0; i < 32; i++)
    {
        std::cout << Register::regNames[i] << ": " << reg.read(i) << std::endl;
    }
    std::cout << "pc: " << reg.getPC() << std::endl;
};

void ISA_Wrapper::reset() { reset_handler.reset(); };

void ISA_Wrapper::load_img()
{
    auto& img = reset_handler.get_img();
    for (int i = 0; i < 5; i++)
    {
        mem.write(i * sizeof(word_t), img[i], sizeof(word_t));
    }
};
