#ifndef ISA_H_
#define ISA_H_

#include "ISA/riscv32/Instruction.h"
#include "ISA/riscv32/Register.h"
#include "ISA/riscv32/Reset.h"
#include "Memory/Memory.h"
#include "Utils/Utils.h"

class ISA_Wrapper
{
   public:
    ISA_Wrapper(Memory& mem, std::function<void(word_t)> ebreak_handler,
                std::function<void(word_t)> invalid_inst_handler);
    ~ISA_Wrapper();
    void execute_one_inst();
    void display_reg();
    void load_img();
    void reset();
    Register& getReg();

   private:
    Memory& mem;
    Register reg;
    Instruction executor;
    Reset_Handler reset_handler;
};

#endif  // ISA_H_