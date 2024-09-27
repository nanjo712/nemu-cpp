#ifndef RISCV32_COMMON_H_
#define RISCV32_COMMON_H_

#include <cstdint>

namespace RISCV32
{

using word_t = uint32_t;
using sword_t = int32_t;

enum InstructionType
{
    R_TYPE,
    I_TYPE,
    S_TYPE,
    B_TYPE,
    U_TYPE,
    J_TYPE
};

template <InstructionType type>
struct InstructionText;

template <>
struct InstructionText<R_TYPE>
{
    static constexpr InstructionType type = R_TYPE;
    word_t opcode : 7;
    word_t rd : 5;
    word_t funct3 : 3;
    word_t rs1 : 5;
    word_t rs2 : 5;
    word_t funct7 : 7;
};

template <>
struct InstructionText<I_TYPE>
{
    static constexpr InstructionType type = I_TYPE;
    word_t opcode : 7;
    word_t rd : 5;
    word_t funct3 : 3;
    word_t rs1 : 5;
    sword_t imm : 12;
};

template <>
struct InstructionText<S_TYPE>
{
    static constexpr InstructionType type = S_TYPE;
    word_t opcode : 7;
    word_t imm4_0 : 5;
    word_t funct3 : 3;
    word_t rs1 : 5;
    word_t rs2 : 5;
    word_t imm11_5 : 7;
};

template <>
struct InstructionText<B_TYPE>
{
    static constexpr InstructionType type = B_TYPE;
    word_t opcode : 7;
    word_t imm11 : 1;
    word_t imm4_1 : 4;
    word_t funct3 : 3;
    word_t rs1 : 5;
    word_t rs2 : 5;
    word_t imm10_5 : 6;
    word_t imm12 : 1;
};

template <>
struct InstructionText<U_TYPE>
{
    static constexpr InstructionType type = U_TYPE;
    word_t opcode : 7;
    word_t rd : 5;
    word_t imm : 20;
};

template <>
struct InstructionText<J_TYPE>
{
    static constexpr InstructionType type = J_TYPE;
    word_t opcode : 7;
    word_t rd : 5;
    word_t imm19_12 : 8;
    word_t imm11 : 1;
    word_t imm10_1 : 10;
    word_t imm20 : 1;
};

union UnionInstructionText
{
    word_t inst_text;
    InstructionText<R_TYPE> r_inst;
    InstructionText<I_TYPE> i_inst;
    InstructionText<S_TYPE> s_inst;
    InstructionText<B_TYPE> b_inst;
    InstructionText<U_TYPE> u_inst;
    InstructionText<J_TYPE> j_inst;
};

enum OpcodeMap
{
    LUI = 0b0110111,
    AUIPC = 0b0010111,
    JAL = 0b1101111,
    JALR = 0b1100111,
    BRANCH = 0b1100011,
    LOAD = 0b0000011,
    STORE = 0b0100011,
    OP_IMM = 0b0010011,
    OP = 0b0110011,
    MISC_MEM = 0b0001111,
    SYSTEM = 0b1110011
};

}  // namespace RISCV32

#endif  // RISCV32_COMMON_H_