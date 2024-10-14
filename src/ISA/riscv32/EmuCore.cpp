#include "ISA/riscv32/EmuCore.hpp"

#include <spdlog/spdlog.h>

#include <print>

#include "Exception/NEMUException.hpp"
#include "ISA/riscv32/Common.hpp"
#include "Utils/Disasm.h"
#include "Utils/Utils.h"

namespace RISCV32
{

std::function<bool()> branch_handler(word_t& src1, word_t& src2, word_t func3)
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
            {
                dest = sign_extend(memory.read(src1 + imm, 1), 8);
#ifdef TRACE_MEMORY
                spdlog::info("Load byte: Addr 0x{:08x} -> Data 0x{:08x}",
                             src1 + imm, dest);
#endif
            };
        case LH:
            return [&memory, &dest, &src1, imm]()
            {
                dest = sign_extend(memory.read(src1 + imm, 2), 16);
#ifdef TRACE_MEMORY
                spdlog::info("Load half word: Addr 0x{:08x} -> Data 0x{:08x}",
                             src1 + imm, dest);
#endif
            };
        case LW:
            return [&memory, &dest, &src1, imm]()
            {
                dest = memory.read(src1 + imm, 4);
#ifdef TRACE_MEMORY
                spdlog::info("Load word: Addr 0x{:08x} -> Data 0x{:08x}",
                             src1 + imm, dest);
#endif
            };
        case LBU:
            return [&memory, &dest, &src1, imm]()
            {
                dest = memory.read(src1 + imm, 1);
#ifdef TRACE_MEMORY
                spdlog::info(
                    "Load unsigned byte: Addr 0x{:08x} -> Data 0x{:08x}",
                    src1 + imm, dest);
#endif
            };
        case LHU:
            return [&memory, &dest, &src1, imm]()
            {
                dest = memory.read(src1 + imm, 2);
#ifdef TRACE_MEMORY
                spdlog::info("Load unsigned half word: Addr 0x{:08x} -> x{}",
                             src1 + imm, dest);
#endif
            };
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
            {
                memory.write(src1 + imm, src2, 1);
#ifdef TRACE_MEMORY
                spdlog::info("Store byte: Addr 0x{:08x} <- Data 0x{:08x}",
                             src1 + imm, src2);
#endif
            };
        case SH:
            return [&memory, &src1, &src2, imm]()
            {
                memory.write(src1 + imm, src2, 2);
#ifdef TRACE_MEMORY
                spdlog::info("Store half word: Addr 0x{:08x} <- Data 0x{:08x}",
                             src1 + imm, src2);
#endif
            };
        case SW:
            return [&memory, &src1, &src2, imm]()
            {
                memory.write(src1 + imm, src2, 4);
#ifdef TRACE_MEMORY
                spdlog::info("Store word: Addr 0x{:08x} <- Data 0x{:08x}",
                             src1 + imm, src2);
#endif
            };
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
            if (imm & 0b010000000000)  // SRAI
                return [&dest, &src1, imm]()
                { dest = static_cast<sword_t>(src1) >> imm; };
            else  // SRLI
                return [&dest, &src1, imm]() { dest = src1 >> imm; };
        default:
            throw invalid_instruction();
    }
}

std::function<void()> RV32I_OP_handler(word_t& dest, word_t& src1, word_t& src2,
                                       word_t func3, word_t func7)
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
        AND = 0b111,
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

std::function<void()> RV32M_OP_handler(word_t& dest, word_t& src1, word_t& src2,
                                       word_t func3)
{
    enum OpFunc3
    {
        MUL = 0b000,
        MULH = 0b001,
        MULHSU = 0b010,
        MULHU = 0b011,
        DIV = 0b100,
        DIVU = 0b101,
        REM = 0b110,
        REMU = 0b111,
    };
    switch (func3)
    {
        case MUL:
            return [&dest, &src1, &src2]()
            {
                int32_t src1_val = src1;
                int32_t src2_val = src2;
                dest = static_cast<sword_t>(src1_val) *
                       static_cast<sword_t>(src2_val);
            };
        case MULH:
            return [&dest, &src1, &src2]()
            {
                int32_t src1_val = src1;
                int32_t src2_val = src2;
                int64_t res = static_cast<int64_t>(src1_val) *
                              static_cast<int64_t>(src2_val);
                dest = res >> 32;
            };
        case MULHSU:
            return [&dest, &src1, &src2]()
            {
                int32_t src1_val = src1;
                uint32_t src2_val = src2;
                int64_t res = static_cast<int64_t>(src1_val) *
                              static_cast<int64_t>(src2_val);
                dest = res >> 32;
            };
        case MULHU:
            return [&dest, &src1, &src2]()
            {
                uint32_t src1_val = src1;
                uint32_t src2_val = src2;
                uint64_t res = static_cast<uint64_t>(src1_val) *
                               static_cast<uint64_t>(src2_val);
                dest = res >> 32;
            };
        case DIV:
            return [&dest, &src1, &src2]()
            {
                int32_t src1_val = src1;
                int32_t src2_val = src2;
                dest = src1_val / src2_val;
            };
        case DIVU:
            return [&dest, &src1, &src2]()
            {
                uint32_t src1_val = src1;
                uint32_t src2_val = src2;
                dest = src1_val / src2_val;
            };
        case REM:
            return [&dest, &src1, &src2]()
            {
                int32_t src1_val = src1;
                int32_t src2_val = src2;
                dest = src1_val % src2_val;
            };
        case REMU:
            return [&dest, &src1, &src2]()
            {
                uint32_t src1_val = src1;
                uint32_t src2_val = src2;
                dest = src1_val % src2_val;
            };
        default:
            throw invalid_instruction();
    }
}

std::function<void()> op_handler(word_t& dest, word_t& src1, word_t& src2,
                                 word_t func7, word_t func3)
{
    enum OpFunc7
    {
        RV32I = 0b0000000,
        RV32ISub = 0b0100000,
        RV32M = 0b0000001,
    };
    switch (func7)
    {
        case RV32M:
            return RV32M_OP_handler(dest, src1, src2, func3);
        case RV32I:
            return RV32I_OP_handler(dest, src1, src2, func3, func7);
        case RV32ISub:
            return RV32I_OP_handler(dest, src1, src2, func3, func7);
        default:
            throw invalid_instruction();
    }
}

EmuCore::RegisterFile::RegisterFile() { x[0] = 0; }

void EmuCore::RegisterFile::reset() { x[0] = 0; }

EmuCore::EmuCore(Memory& memory) : memory(memory), null_operand(0), pc(pc_init)
{
    init_disasm("riscv32-pc-linux-gnu");
}

EmuCore::~EmuCore() {}

word_t EmuCore::imm_generate(word_t inst, InstructionType type)
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

EmuCore::Handler EmuCore::decode(word_t inst, word_t& next_pc)
{
    UnionInstructionText inst_text({.inst_text = inst});
    auto opcode = static_cast<OpcodeMap>(inst_text.r_inst.opcode);
    switch (opcode)
    {
        case OpcodeMap::LUI:
        {
            auto& dest = register_file.x.at(inst_text.u_inst.rd);
            auto imm = imm_generate(inst, U_TYPE);
            return [&dest, imm]() { dest = imm; };
        }
        case OpcodeMap::AUIPC:
        {
            auto& dest = register_file.x.at(inst_text.u_inst.rd);
            auto imm = imm_generate(inst, U_TYPE) + pc;
            return [&dest, imm]() { dest = imm; };
        }
        case OpcodeMap::JAL:
        {
            auto& dest = register_file.x.at(inst_text.j_inst.rd);
            auto& pc = this->pc;
            auto imm = imm_generate(inst, J_TYPE);
            return [&dest, &pc, &next_pc, imm]()
            {
                dest = next_pc;
                next_pc = pc + imm;
            };
        }
        case OpcodeMap::JALR:
        {
            auto& dest = register_file.x.at(inst_text.i_inst.rd);
            auto& src1 = register_file.x.at(inst_text.i_inst.rs1);
            auto imm = imm_generate(inst, I_TYPE);
            return [&dest, &src1, &next_pc, imm]()
            {
                dest = next_pc;
                next_pc = (src1 + imm) & ~1;
            };
        }
        case OpcodeMap::BRANCH:
        {
            auto& src1 = register_file.x.at(inst_text.b_inst.rs1);
            auto& src2 = register_file.x.at(inst_text.b_inst.rs2);
            auto& pc = this->pc;
            auto imm = imm_generate(inst, B_TYPE);
            auto compare = branch_handler(src1, src2, inst_text.b_inst.funct3);
            return [&pc, &next_pc, imm, compare]()
            {
                if (compare()) next_pc = pc + imm;
            };
        }
        case OpcodeMap::LOAD:
        {
            auto& dest = register_file.x.at(inst_text.i_inst.rd);
            auto& src1 = register_file.x.at(inst_text.i_inst.rs1);
            auto imm = imm_generate(inst, I_TYPE);
            return load_handler(dest, src1, imm, inst_text.i_inst.funct3,
                                memory);
        }
        case OpcodeMap::STORE:
        {
            auto& src1 = register_file.x.at(inst_text.s_inst.rs1);
            auto& src2 = register_file.x.at(inst_text.s_inst.rs2);
            auto imm = imm_generate(inst, S_TYPE);
            return store_handler(src1, src2, imm, inst_text.s_inst.funct3,
                                 memory);
        }
        case OpcodeMap::OP_IMM:
        {
            auto& dest = register_file.x.at(inst_text.i_inst.rd);
            auto& src1 = register_file.x.at(inst_text.i_inst.rs1);
            auto imm = imm_generate(inst, I_TYPE);
            return op_imm_handler(dest, src1, imm, inst_text.i_inst.funct3);
        }
        case OpcodeMap::OP:
        {
            auto& dest = register_file.x.at(inst_text.r_inst.rd);
            auto& src1 = register_file.x.at(inst_text.r_inst.rs1);
            auto& src2 = register_file.x.at(inst_text.r_inst.rs2);
            return op_handler(dest, src1, src2, inst_text.r_inst.funct7,
                              inst_text.r_inst.funct3);
        }
        case OpcodeMap::SYSTEM:
        {
            auto imm = imm_generate(inst, I_TYPE);
            if (imm == 1)
            {
                return []() { throw ebreak_exception(); };
            }
        }
        case OpcodeMap::MISC_MEM:
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
    auto next_pc = pc + 4;
    auto handler = decode(inst, next_pc);
    handler();
    pc = next_pc;
    register_file.x[0] = 0;
}

word_t EmuCore::debug_get_reg_val_impl(int reg_num)
{
    return register_file.x.at(reg_num);
}

word_t EmuCore::debug_get_pc_impl() { return pc; }

word_t EmuCore::debug_get_reg_index_impl(std::string_view reg_name)
{
    for (int i = 0; i < 32; i++)
        if (reg_name == RISCV32::reg_name_list[i]) return i;
    return -1;
}

}  // namespace RISCV32