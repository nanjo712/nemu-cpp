#ifndef ISA_H_
#define ISA_H_

#include "ISA/riscv32/Instruction.h"
#include "ISA/riscv32/Register.h"
#include "ISA/riscv32/Reset.h"
#include "Memory/Memory.h"

class ISA_Wrapper
{
   public:
    ISA_Wrapper(Memory& mem);
    ~ISA_Wrapper();
    void execute_one_inst();
    void display_reg();
    void load_img();
    void reset();

   private:
    Memory& mem;
    Register reg;
    Instruction executor;
    Reset_Handler reset_handler;
};

#endif  // ISA_H_