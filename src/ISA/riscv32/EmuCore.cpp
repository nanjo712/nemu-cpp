#include "ISA/riscv32/EmuCore.hpp"

#include "Exception/NEMUException.hpp"
#include "ISA/riscv32/Common.hpp"
#include "Utils/Utils.h"

namespace RISCV32
{

std::function<bool()> branch_handler(word_t src1, word_t src2, word_t func3)
{
    enum BranchFunc3
    {
        BEQ = 0b000,
        BNE = 0b001,
        BLT = 0b100,
        BGE = 0b101,
        BLTU = 0b110,
        BGEU = 0b111
    };
    switch (func3)
    {
        case BEQ:
            return [&src1, &src2]() { return src1 == src2; };
        case BNE:
            return [&src1, &src2]() { return src1 != src2; };
        case BLT:
            return [&src1, &src2]()
            { return static_cast<sword_t>(src1) < static_cast<sword_t>(src2); };
        case BGE:
            return [&src1, &src2]() {
                return static_cast<sword_t>(src1) >= static_cast<sword_t>(src2);
            };
        case BLTU:
            return [&src1, &src2]() { return src1 < src2; };
        case BGEU:
            return [&src1, &src2]() { return src1 >= src2; };
        default:
            throw invalid_instruction();
    }
}

std::function<void()> load_handler(word_t& dest, word_t& src1, word_t imm,
                                   word_t func3, Memory& memory)
{
    enum LoadFunc3
    {
        LB = 0b000,
        LH = 0b001,
        LW = 0b010,
        LBU = 0b100,
        LHU = 0b101
    };
    switch (func3)
    {
        case LB:
            return [&memory, &dest, &src1, imm]()
            { dest = sign_extend(memory.read(src1 + imm, 1), 8); };
        case LH:
            return [&memory, &dest, &src1, imm]()
            { dest = sign_extend(memory.read(src1 + imm, 2), 16); };
        case LW:
            return [&memory, &dest, &src1, imm]()
            { dest = memory.read(src1 + imm, 4); };
        case LBU:
            return [&memory, &dest, &src1, imm]()
            { dest = memory.read(src1 + imm, 1); };
        case LHU:
            return [&memory, &dest, &src1, imm]()
            { dest = memory.read(src1 + imm, 2); };
        default:
            throw invalid_instruction();
    }
}

std::function<void()> store_handler(word_t& src1, word_t& src2, word_t imm,
                                    word_t func3, Memory& memory)
{
    enum StoreFunc3
    {
        SB = 0b000,
        SH = 0b001,
        SW = 0b010
    };
    switch (func3)
    {
        case SB:
            return [&memory, &src1, &src2, imm]()
            { memory.write(src1 + imm, src2, 1); };
        case SH:
            return [&memory, &src1, &src2, imm]()
            { memory.write(src1 + imm, src2, 2); };
        case SW:
            return [&memory, &src1, &src2, imm]()
            { memory.write(src1 + imm, src2, 4); };
        default:
            throw invalid_instruction();
    }
}

std::function<void()> op_imm_handler(word_t& dest, word_t& src1, word_t imm,
                                     word_t func3)
{
    enum OpImmFunc3
    {
        ADDI = 0b000,
        SLTI = 0b010,
        SLTIU = 0b011,
        XORI = 0b100,
        ORI = 0b110,
        ANDI = 0b111,
        SLLI = 0b001,
        SRLIorSRAI = 0b101,  // SRLI or SRAI
    };
    switch (func3)
    {
        case ADDI:
            return [&dest, &src1, imm]() { dest = src1 + imm; };
        case SLTI:
            return [&dest, &src1, imm]()
            { dest = static_cast<sword_t>(src1) < static_cast<sword_t>(imm); };
        case SLTIU:
            return [&dest, &src1, imm]() { dest = src1 < imm; };
        case XORI:
            return [&dest, &src1, imm]() { dest = src1 ^ imm; };
        case ORI:
            return [&dest, &src1, imm]() { dest = src1 | imm; };
        case ANDI:
            return [&dest, &src1, imm]() { dest = src1 & imm; };
        case SLLI:
            return [&dest, &src1, imm]() { dest = src1 << imm; };
        case SRLIorSRAI:
            if (imm & 0b0100000)
                return [&dest, &src1, imm]()
                { dest = static_cast<sword_t>(src1) >> imm; };
            else
                return [&dest, &src1, imm]() { dest = src1 >> imm; };
        default:
            throw invalid_instruction();
    }
}

std::function<void()> op_handler(word_t& dest, word_t& src1, word_t& src2,
                                 word_t func7, word_t func3)
{
    enum OpFunc3
    {
        ADDorSUB = 0b000,
        SLL = 0b001,
        SLT = 0b010,
        SLTU = 0b011,
        XOR = 0b100,
        SRLorSRA = 0b101,
        OR = 0b110,
        AND = 0b111
    };
    switch (func3)
    {
        case ADDorSUB:
            if (func7 == 0)
                return [&dest, &src1, &src2]() { dest = src1 + src2; };
            else
                return [&dest, &src1, &src2]() { dest = src1 - src2; };
        case SLL:
            return [&dest, &src1, &src2]() { dest = src1 << src2; };
        case SLT:
            return [&dest, &src1, &src2]()
            { dest = static_cast<sword_t>(src1) < static_cast<sword_t>(src2); };
        case SLTU:
            return [&dest, &src1, &src2]() { dest = src1 < src2; };
        case XOR:
            return [&dest, &src1, &src2]() { dest = src1 ^ src2; };
        case SRLorSRA:
            if (func7 == 0)
                return [&dest, &src1, &src2]() { dest = src1 >> src2; };
            else
                return [&dest, &src1, &src2]()
                { dest = static_cast<sword_t>(src1) >> src2; };
        case OR:
            return [&dest, &src1, &src2]() { dest = src1 | src2; };
        case AND:
            return [&dest, &src1, &src2]() { dest = src1 & src2; };
        default:
            throw invalid_instruction();
    }
}

EmuCore::RegisterFile::RegisterFile() { x[0] = 0; }

EmuCore::EmuCore(Memory& memory) : memory(memory), null_operand(0), pc(pc_init)
{
}

EmuCore::~EmuCore() {}

word_t EmuCore::immGenerate(word_t inst, InstructionType type)
{
    switch (type)
    {
        case I_TYPE:
            return sign_extend(extract_bits(inst, 20, 31), 12);
        case S_TYPE:
            return sign_extend(
                (extract_bits(inst, 25, 31) << 5) | extract_bits(inst, 7, 11),
                12);
        case B_TYPE:
            return sign_extend((extract_bits(inst, 31, 31) << 12) |
                                   (extract_bits(inst, 7, 7) << 11) |
                                   (extract_bits(inst, 25, 30) << 5) |
                                   (extract_bits(inst, 8, 11) << 1),
                               13);
        case U_TYPE:
            return extract_bits(inst, 12, 31) << 12;
        case J_TYPE:
            return sign_extend((extract_bits(inst, 31, 31) << 20) |
                                   (extract_bits(inst, 21, 30) << 1) |
                                   (extract_bits(inst, 20, 20) << 11) |
                                   (extract_bits(inst, 12, 19) << 12),
                               21);
        default:
            throw invalid_instruction();
    }
}

EmuCore::Handler EmuCore::decode(word_t inst)
{
    UnionInstructionText inst_text({.inst_text = inst});
    auto opcode = static_cast<OpcodeMap>(inst_text.r_inst.opcode);
    switch (opcode)
    {
        case OpcodeMap::LUI:
        {
            auto& dest = register_file.x[inst_text.u_inst.rd];
            auto imm = immGenerate(inst, U_TYPE);
            return [&dest, imm]() { dest = imm; };
        }
        case OpcodeMap::AUIPC:
        {
            auto& dest = register_file.x[inst_text.u_inst.rd];
            auto imm = immGenerate(inst, U_TYPE) + pc;
            return [&dest, imm]() { dest = imm; };
        }
        case OpcodeMap::JAL:
        {
            auto& dest = register_file.x[inst_text.j_inst.rd];
            auto& pc = this->pc;
            auto imm = immGenerate(inst, J_TYPE);
            return [&dest, &pc, imm]()
            {
                dest = pc + 4;
                pc += imm;
            };
        }
        case OpcodeMap::JALR:
        {
            auto& dest = register_file.x[inst_text.i_inst.rd];
            auto& src1 = register_file.x[inst_text.i_inst.rs1];
            auto& pc = this->pc;
            auto imm = immGenerate(inst, I_TYPE);
            return [&dest, &src1, &pc, imm]()
            {
                dest = pc + 4;
                pc = (src1 + imm) & ~1;
            };
        }
        case OpcodeMap::BRANCH:
        {
            auto& src1 = register_file.x[inst_text.b_inst.rs1];
            auto& src2 = register_file.x[inst_text.b_inst.rs2];
            auto& pc = this->pc;
            auto imm = immGenerate(inst, B_TYPE);
            auto compare = branch_handler(src1, src2, inst_text.b_inst.funct3);
            return [&pc, imm, compare]()
            {
                if (compare())
                    pc += imm;
                else
                    pc += 4;
            };
        }
        case OpcodeMap::LOAD:
        {
            auto& dest = register_file.x[inst_text.i_inst.rd];
            auto& src1 = register_file.x[inst_text.i_inst.rs1];
            auto imm = immGenerate(inst, I_TYPE);
            return load_handler(dest, src1, imm, inst_text.i_inst.funct3,
                                memory);
        }
        case OpcodeMap::STORE:
        {
            auto& src1 = register_file.x[inst_text.s_inst.rs1];
            auto& src2 = register_file.x[inst_text.s_inst.rs2];
            auto imm = immGenerate(inst, S_TYPE);
            return store_handler(src1, src2, imm, inst_text.s_inst.funct3,
                                 memory);
        }
        case OpcodeMap::OP_IMM:
        {
            auto& dest = register_file.x[inst_text.i_inst.rd];
            auto& src1 = register_file.x[inst_text.i_inst.rs1];
            auto imm = immGenerate(inst, I_TYPE);
            return op_imm_handler(dest, src1, imm, inst_text.i_inst.funct3);
        }
        case OpcodeMap::OP:
        {
            auto& dest = register_file.x[inst_text.r_inst.rd];
            auto& src1 = register_file.x[inst_text.r_inst.rs1];
            auto& src2 = register_file.x[inst_text.r_inst.rs2];
            return op_handler(dest, src1, src2, inst_text.r_inst.funct7,
                              inst_text.r_inst.funct3);
        }
        case OpcodeMap::MISC_MEM:
        case OpcodeMap::SYSTEM:
        default:
            throw invalid_instruction();
    }
}

void EmuCore::reset_impl()
{
    pc = pc_init;
    register_file.reset();
}

void EmuCore::single_instruction_impl()
{
    auto inst = memory.read(pc, 4);
    auto handler = decode(inst);
    handler();
}

}  // namespace RISCV32