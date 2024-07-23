#ifndef ISA_H_
#define ISA_H_

#include "ISA/riscv32/Instruction.h"
#include "ISA/riscv32/Register.h"
#include "ISA/riscv32/Reset.h"
#include "Memory/Memory.h"

class ISA_Wrapper
{
   public:
    Register reg;
    Instruction executor;
    Reset_Handler reset_handler;

    ISA_Wrapper(const ISA_Wrapper&) = delete;
    ISA_Wrapper& operator=(const ISA_Wrapper&) = delete;
    ISA_Wrapper(ISA_Wrapper&&) = delete;
    ISA_Wrapper& operator=(ISA_Wrapper&&) = delete;

    ~ISA_Wrapper();

    static ISA_Wrapper& getISA();
    void execute_one_inst();
    void display_reg();
    void load_img();

   private:
    Memory& mem;

    ISA_Wrapper();
};

#endif  // ISA_H_