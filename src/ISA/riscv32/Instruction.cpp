#include "ISA/riscv32/Instruction.h"

#include <fmt/core.h>

#include <cassert>
#include <iostream>

#include "ISA/riscv32/Register.h"
#include "Memory/Memory.h"
#include "Monitor/Monitor.h"
#include "Utils/Utils.h"

Instruction::Instruction(Register &reg)
    : reg(reg),
      mem(Memory::getMemory()),
      instList({
          InstInfo{"??????? ????? ????? ??? ????? 01101 11", "lui", U,
                   [this]() { lui(); }},
          InstInfo{"??????? ????? ????? ??? ????? 00101 11", "auipc", U,
                   [this]() { auipc(); }},
          InstInfo{"??????? ????? ????? ??? ????? 11011 11", "jal", U,
                   [this]() { jal(); }},
          InstInfo{"??????? ????? ????? 000 ????? 11001 11", "jalr", I,
                   [this]() { jalr(); }},
          InstInfo{"??????? ????? ????? 000 ????? 11000 11", "beq", B,
                   [this]() { beq(); }},
          InstInfo{"??????? ????? ????? 001 ????? 11000 11", "bne", B,
                   [this]() { bne(); }},
          InstInfo{"??????? ????? ????? 100 ????? 11000 11", "blt", B,
                   [this]() { blt(); }},
          InstInfo{"??????? ????? ????? 101 ????? 11000 11", "bge", B,
                   [this]() { bge(); }},
          InstInfo{"??????? ????? ????? 110 ????? 11000 11", "bltu", B,
                   [this]() { bltu(); }},
          InstInfo{"??????? ????? ????? 111 ????? 11000 11", "bgeu", B,
                   [this]() { bgeu(); }},
          //   InstInfo{"??????? ????? ????? 000 ????? 00000 11", "lb", I,
          //            [this]() { lb(); }},
          InstInfo{"??????? ????? ????? 001 ????? 00000 11", "lh", I,
                   [this]() { lh(); }},
          InstInfo{"??????? ????? ????? 010 ????? 00000 11", "lw", I,
                   [this]() { lw(); }},
          InstInfo{"??????? ????? ????? 100 ????? 00000 11", "lbu", I,
                   [this]() { lbu(); }},
          InstInfo{"??????? ????? ????? 101 ????? 00000 11", "lhu", I,
                   [this]() { lhu(); }},
          InstInfo{"??????? ????? ????? 000 ????? 01000 11", "sb", S,
                   [this]() { sb(); }},
          InstInfo{"??????? ????? ????? 001 ????? 01000 11", "sh", S,
                   [this]() { sh(); }},
          InstInfo{"??????? ????? ????? 010 ????? 01000 11", "sw", S,
                   [this]() { sw(); }},
          InstInfo{"??????? ????? ????? 000 ????? 00100 11", "addi", I,
                   [this]() { addi(); }},
          //   InstInfo{"??????? ????? ????? 010 ????? 00100 11", "slti", I,
          //            [this]() { slti(); }},
          InstInfo{"??????? ????? ????? 011 ????? 00100 11", "sltiu", I,
                   [this]() { sltiu(); }},
          InstInfo{"??????? ????? ????? 100 ????? 00100 11", "xori", I,
                   [this]() { xori(); }},
          //   InstInfo{"??????? ????? ????? 110 ????? 00100 11", "ori", I,
          //            [this]() { ori(); }},
          InstInfo{"??????? ????? ????? 111 ????? 00100 11", "andi", I,
                   [this]() { andi(); }},
          InstInfo{"0000000 ????? ????? 001 ????? 00100 11", "slli", I,
                   [this]() { slli(); }},
          InstInfo{"0000000 ????? ????? 101 ????? 00100 11", "srli", I,
                   [this]() { srli(); }},
          InstInfo{"0100000 ????? ????? 101 ????? 00100 11", "srai", I,
                   [this]() { srai(); }},
          InstInfo{"0000000 ????? ????? 000 ????? 01100 11", "add", R,
                   [this]() { add(); }},
          InstInfo{"0100000 ????? ????? 000 ????? 01100 11", "sub", R,
                   [this]() { sub(); }},
          InstInfo{"0000000 ????? ????? 001 ????? 01100 11", "sll", R,
                   [this]() { sll(); }},
          //   InstInfo{"0000000 ????? ????? 010 ????? 01100 11", "slt", R,
          //            [this]() { slt(); }},
          InstInfo{"0000000 ????? ????? 011 ????? 01100 11", "sltu", R,
                   [this]() { sltu(); }},
          InstInfo{"0000000 ????? ????? 100 ????? 01100 11", "xor", R,
                   [this]() { xor_(); }},
          InstInfo{"0000000 ????? ????? 101 ????? 01100 11", "srl", R,
                   [this]() { srl(); }},
          InstInfo{"0100000 ????? ????? 101 ????? 01100 11", "sra", R,
                   [this]() { sra(); }},
          InstInfo{"0000000 ????? ????? 110 ????? 01100 11", "or", R,
                   [this]() { or_(); }},
          InstInfo{"0000000 ????? ????? 111 ????? 01100 11", "and", R,
                   [this]() { and_(); }},
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

void Instruction::lui() { reg.write(rd, immU); }

void Instruction::auipc() { reg.write(rd, reg.getPC() + immU); }

void Instruction::jal()
{
    reg.write(rd, nextPC);
    nextPC = reg.getPC() + immJ;
}

void Instruction::jalr()
{
    reg.write(rd, nextPC);
    nextPC = (reg.read(rs1) + immI) & ~1;
}

void Instruction::beq()
{
    if (reg.read(rs1) == reg.read(rs2))
    {
        nextPC = reg.getPC() + immB;
    }
}

void Instruction::bne()
{
    if (reg.read(rs1) != reg.read(rs2))
    {
        nextPC = reg.getPC() + immB;
    }
}

void Instruction::blt()
{
    sword_t rs1_val = reg.read(rs1);
    sword_t rs2_val = reg.read(rs2);
    if (rs1_val < rs2_val)
    {
        nextPC = reg.getPC() + immB;
    }
}

void Instruction::bge()
{
    sword_t rs1_val = reg.read(rs1);
    sword_t rs2_val = reg.read(rs2);
    if (rs1_val >= rs2_val)
    {
        nextPC = reg.getPC() + immB;
    }
}

void Instruction::bltu()
{
    if (reg.read(rs1) < reg.read(rs2))
    {
        nextPC = reg.getPC() + immB;
    }
}

void Instruction::bgeu()
{
    if (reg.read(rs1) >= reg.read(rs2))
    {
        nextPC = reg.getPC() + immB;
    }
}

void Instruction::lb()
{
    reg.write(rd, sign_extend(mem.read(reg.read(rs1) + immI, 1), 8));
}

void Instruction::lh()
{
    reg.write(rd, sign_extend(mem.read(reg.read(rs1) + immI, 2), 16));
}

void Instruction::lw() { reg.write(rd, mem.read(reg.read(rs1) + immI, 4)); }

void Instruction::lbu() { reg.write(rd, mem.read(reg.read(rs1) + immI, 1)); }

void Instruction::lhu() { reg.write(rd, mem.read(reg.read(rs1) + immI, 2)); }

void Instruction::sb() { mem.write(reg.read(rs1) + immS, reg.read(rs2), 1); }

void Instruction::sh() { mem.write(reg.read(rs1) + immS, reg.read(rs2), 2); }

void Instruction::sw() { mem.write(reg.read(rs1) + immS, reg.read(rs2), 4); }

void Instruction::addi() { reg.write(rd, reg.read(rs1) + immI); }

void Instruction::slti()
{
    sword_t rs1_val = reg.read(rs1);
    reg.write(rd, rs1_val < static_cast<sword_t>(immI));
}

void Instruction::sltiu()
{
    reg.write(rd, reg.read(rs1) < static_cast<word_t>(immI));
}

void Instruction::xori() { reg.write(rd, reg.read(rs1) ^ immI); }

void Instruction::ori() { reg.write(rd, reg.read(rs1) | immI); }

void Instruction::andi() { reg.write(rd, reg.read(rs1) & immI); }

void Instruction::slli() { reg.write(rd, reg.read(rs1) << immI); }

void Instruction::srli() { reg.write(rd, reg.read(rs1) >> immI); }

void Instruction::srai()
{
    sword_t rs1_val = reg.read(rs1);
    sword_t shamt = immI & 0x1f;
    reg.write(rd, rs1_val >> shamt);
}

void Instruction::add() { reg.write(rd, reg.read(rs1) + reg.read(rs2)); }

void Instruction::sub() { reg.write(rd, reg.read(rs1) - reg.read(rs2)); }

void Instruction::sll()
{
    reg.write(rd, reg.read(rs1) << (reg.read(rs2) & 0x1f));
}

void Instruction::slt()
{
    sword_t rs1_val = reg.read(rs1);
    sword_t rs2_val = reg.read(rs2);
    reg.write(rd, rs1_val < rs2_val);
}

void Instruction::sltu() { reg.write(rd, reg.read(rs1) < reg.read(rs2)); }

void Instruction::xor_() { reg.write(rd, reg.read(rs1) ^ reg.read(rs2)); }

void Instruction::srl()
{
    reg.write(rd, reg.read(rs1) >> (reg.read(rs2) & 0x1f));
}

void Instruction::sra()
{
    sword_t rs1_val = reg.read(rs1);
    sword_t rs2_val = reg.read(rs2);
    reg.write(rd, rs1_val >> (rs2_val & 0x1f));
}

void Instruction::or_() { reg.write(rd, reg.read(rs1) | reg.read(rs2)); }

void Instruction::and_() { reg.write(rd, reg.read(rs1) & reg.read(rs2)); }

void Instruction::ebreak()
{
    Monitor::getMonitor().ebreak_handler(reg.getPC());
}

void Instruction::inv()
{
    std::cout << fmt::format("Opcode: {:#b}\n", opcode);
    std::cout << fmt::format("func3: {:#b}\n", funct3);
    std::cout << fmt::format("func7: {:#b}\n", funct7);
    Monitor::getMonitor().invalid_inst_handler(reg.getPC());
}
