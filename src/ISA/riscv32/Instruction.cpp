#include "ISA/riscv32/Instruction.h"

#include <cassert>

#include "ISA/riscv32/Register.h"
#include "Memory/Memory.h"

Instruction::Instruction(Register &reg, Memory &mem)
    : reg(reg),
      mem(mem),
      instList({
          InstInfo{"??????? ????? ????? ??? ????? 00101 11", "auipc", U,
                   [this]() { auipc(); }},
          InstInfo{"??????? ????? ????? 100 ????? 00000 11", "lbu", I,
                   [this]() { lbu(); }},
          InstInfo{"??????? ????? ????? 000 ????? 01000 11", "sb", S,
                   [this]() { sb(); }},
          InstInfo{"0000000 00001 00000 000 00000 11100 11", "ebreak", N,
                   [this]() { ebreak(); }},
          InstInfo{"??????? ????? ????? ??? ????? ????? ??", "inv", N,
                   [this]() { inv(); }},
      })
{
    for (auto &inst : instList)
    {
        inst.mask = 0;
        for (auto c : inst.format)
        {
            if (c == ' ')
            {
                continue;
            }
            inst.mask <<= 1;
            if (c != '?')
            {
                inst.mask |= 1;
            }
        }
    }
}

Instruction::~Instruction() {}

void Instruction::execute(word_t inst)
{
    this->inst = inst;
    this->opcode = extract_bits(inst, 0, 6);
    this->rd = extract_bits(inst, 7, 11);
    this->rs1 = extract_bits(inst, 15, 19);
    this->rs2 = extract_bits(inst, 20, 24);
    this->funct3 = extract_bits(inst, 12, 14);
    this->funct7 = extract_bits(inst, 25, 31);
    this->immI = sign_extend(extract_bits(inst, 20, 31), 12);
    this->immS = sign_extend(
        (extract_bits(inst, 25, 31) << 5) | extract_bits(inst, 7, 11), 12);
    this->immB = sign_extend((extract_bits(inst, 31, 31) << 12) |
                                 (extract_bits(inst, 7, 7) << 11) |
                                 (extract_bits(inst, 25, 30) << 5) |
                                 (extract_bits(inst, 8, 11) << 1),
                             13);
    this->immU = extract_bits(inst, 12, 31) << 12;
    this->immJ = sign_extend((extract_bits(inst, 31, 31) << 20) |
                                 (extract_bits(inst, 21, 30) << 1) |
                                 (extract_bits(inst, 20, 20) << 11) |
                                 (extract_bits(inst, 12, 19) << 12),
                             21);
    word_t nextPC = reg.getPC() + 4;
    execute();
    reg.setPC(nextPC);
}

void Instruction::execute()
{
    for (auto &inst : instList)
    {
        if ((inst.mask & this->inst) == this->inst)
        {
            inst.exec();
            return;
        }
    }
}

void Instruction::auipc() { reg.write(rd, reg.getPC() + immU); }
void Instruction::lbu() { reg.write(rd, mem.read(reg.read(rs1) + immI, 1)); }
void Instruction::sb() { mem.write(reg.read(rs1) + immS, reg.read(rs2), 1); }
void Instruction::ebreak() {}               // Just halt the program
void Instruction::inv() { assert(false); }  // Invalid instruction
