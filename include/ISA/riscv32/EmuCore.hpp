#ifndef EMUCORE_H_
#define EMUCORE_H_

#include <array>
#include <functional>
#include <string_view>

#include "Core/Core.hpp"
#include "ISA/riscv32/Common.hpp"
#include "Memory/Memory.h"

namespace RISCV32
{

class EmuCore : public Core<EmuCore>
{
   public:
    using word_t = RISCV32::word_t;
    using sword_t = RISCV32::sword_t;

    EmuCore(Memory& memory);
    ~EmuCore();

   private:
    static constexpr RISCV32::word_t pc_init = 0x80000000;
    using Handler = std::function<void()>;

    friend class Core<EmuCore>;
    Memory& memory;
    word_t null_operand;
    word_t pc;

    struct RegisterFile
    {
        std::array<word_t, 32> x;
        RegisterFile();
        void reset();
    } register_file;

    word_t immGenerate(word_t inst, InstructionType type);
    Handler decode(word_t inst);

    void reset_impl();
    void single_instruction_impl();
    word_t debug_get_pc_impl();
    word_t debug_get_reg_val_impl(int reg_num);
    word_t debug_get_reg_index_impl(std::string_view reg_name);
};

}  // namespace RISCV32

#endif  // EMUCORE_H_