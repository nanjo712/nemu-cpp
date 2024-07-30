#include "ISA/riscv32/Instruction.h"

#include <cassert>

#include "ISA/riscv32/Register.h"
#include "Memory/Memory.h"
#include "Monitor/Monitor.h"

Instruction::Instruction(Register &reg)
    : reg(reg),
      mem(Memory::getMemory()),
      instList({
          InstInfo{"??????? ????? ????? ??? ????? 00101 11", "auipc", U,
                   [this]() { auipc(); }},
          InstInfo{"??????? ????? ????? ??? ????? 11011 11", "jal", U,
                   [this]() { jal(); }},
          InstInfo{"??????? ????? ????? 000 ????? 11001 11", "jalr", I,
                   [this]() { jalr(); }},
          InstInfo{"??????? ????? ????? 010 ????? 00000 11", "lw", I,
                   [this]() { lw(); }},
          InstInfo{"??????? ????? ????? 100 ????? 00000 11", "lbu", I,
                   [this]() { lbu(); }},
          InstInfo{"??????? ????? ????? 000 ????? 01000 11", "sb", S,
                   [this]() { sb(); }},
          InstInfo{"??????? ????? ????? 010 ????? 01000 11", "sw", S,
                   [this]() { sw(); }},
          InstInfo{"??????? ????? ????? 000 ????? 00100 11", "addi", I,
                   [this]() { addi(); }},
          InstInfo{"0000000 ????? ????? 000 ????? 01100 11", "add", R,
                   [this]() { add(); }},
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
            inst.pattern <<= 1;
            if (c != '?')
            {
                inst.mask |= 1;
                inst.pattern |= c - '0';
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
    nextPC = reg.getPC() + 4;
    execute();
    reg.setPC(nextPC);
}

void Instruction::execute()
{
    for (auto &inst : instList)
    {
        if ((inst.mask & this->inst) == inst.pattern)
        {
            inst.exec();
            return;
        }
    }
}

void Instruction::auipc() { reg.write(rd, reg.getPC() + immU); }

void Instruction::jal()
{
    reg.write(rd, nextPC + 4);
    nextPC = reg.getPC() + immJ;
}

void Instruction::jalr()
{
    reg.write(rd, nextPC);
    nextPC = (reg.read(rs1) + immI) & ~1;
}

void Instruction::lw() { reg.write(rd, mem.read(reg.read(rs1) + immI, 4)); }

void Instruction::lbu() { reg.write(rd, mem.read(reg.read(rs1) + immI, 1)); }

void Instruction::sb() { mem.write(reg.read(rs1) + immS, reg.read(rs2), 1); }

void Instruction::sw() { mem.write(reg.read(rs1) + immS, reg.read(rs2), 4); }

void Instruction::addi() { reg.write(rd, reg.read(rs1) + immI); }

void Instruction::add() { reg.write(rd, reg.read(rs1) + reg.read(rs2)); }

void Instruction::ebreak()
{
    Monitor::getMonitor().ebreak_handler(reg.getPC());
}

void Instruction::inv()
{
    Monitor::getMonitor().invalid_inst_handler(reg.getPC());
}
