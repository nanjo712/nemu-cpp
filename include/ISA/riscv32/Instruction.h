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
    Instruction(Register &reg);
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
        word_t pattern;
    };
    word_t inst, opcode, rd, rs1, rs2, funct3, funct7, immI, immS, immB, immU,
        immJ;
    word_t nextPC;
    Register &reg;
    Memory &mem;
    std::array<Instruction::InstInfo, 128> instList;

    void execute();

    // RV32I
    void lui();
    void auipc();
    void jal();
    void jalr();
    void beq();
    void bne();
    void blt();
    void bge();
    void bltu();
    void bgeu();
    void lb();
    void lh();
    void lw();
    void lbu();
    void lhu();
    void sb();
    void sh();
    void sw();
    void addi();
    void slti();
    void sltiu();
    void xori();
    void ori();
    void andi();
    void slli();
    void srli();
    void srai();
    void add();
    void sub();
    void sll();
    void slt();
    void sltu();
    void xor_();
    void srl();
    void sra();
    void or_();
    void and_();

    // RV32M
    void mul();
    void mulh();
    void mulhsu();
    void mulhu();
    void div();
    void divu();
    void rem();
    void remu();

    void ebreak();
    void inv();
};

#endif  // INSTRUCTION_H_