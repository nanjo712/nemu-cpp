#ifndef ISA_H_
#define ISA_H_

#include "ISA/riscv32/Instruction.h"
#include "ISA/riscv32/Register.h"
#include "ISA/riscv32/Reset.h"

class Memory;

class ISA_Wrapper
{
   private:
    Memory& mem;
    Register reg;
    Instruction executor;
    Reset_Handler reset_handler;

   public:
    ISA_Wrapper(const ISA_Wrapper&) = delete;
    ISA_Wrapper& operator=(const ISA_Wrapper&) = delete;
    ISA_Wrapper(ISA_Wrapper&&) = delete;
    ISA_Wrapper& operator=(ISA_Wrapper&&) = delete;

    ~ISA_Wrapper();

    static ISA_Wrapper& getISA();
    void execute_one_inst();
    void display_reg();
    void load_img();
    word_t get_reg_val(const std::string_view reg_name);

   private:
    ISA_Wrapper();

    friend class Monitor;
};

#endif  // ISA_H_