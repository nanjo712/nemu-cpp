#ifndef EMU_CORE_H_
#define EMU_CORE_H_

#include "Core/Core.hpp"
#include "Memory/Memory.h"

class EmuCore : public Core<EmuCore>
{
   public:
    EmuCore(Memory& mem);
    ~EmuCore();

   private:
    void single_instruction_impl();
    void reset_impl();
    void debug_get_reg_val_impl();

    word_t pc;
    Memory& mem;
    friend Core<EmuCore>;
};

#endif  // EMU_CORE_H_
