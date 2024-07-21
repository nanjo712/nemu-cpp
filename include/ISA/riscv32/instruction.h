#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <array>
#include <functional>
#include <string_view>

#include "Utils/Utils.h"

class Register;
class Memory;

class Instruction
{
   public:
    Instruction(Register &reg, Memory &mem);
    ~Instruction();
    void execute(word_t inst);

   private:
    enum Type
    {
        R,
        I,
        S,
        B,
        U,
        J,
        N
    };
    struct InstInfo
    {
        std::string_view format;
        std::string_view name;
        Type type;
        std::function<void()> exec;
        word_t mask;
    };
    word_t inst, opcode, rd, rs1, rs2, funct3, funct7, immI, immS, immB, immU,
        immJ;
    Register &reg;
    Memory &mem;
    std::array<Instruction::InstInfo, 128> instList;

    void execute();
    void auipc();
    void lbu();
    void sb();
    void ebreak();
    void inv();
};

#endif  // INSTRUCTION_H_