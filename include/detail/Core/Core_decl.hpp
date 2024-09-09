#ifndef CORE_DECL_H_
#define CORE_DECL_H_

#include "Utils/Utils.h"

template <typename T>
class Core
{
   public:
    Core();
    ~Core();

    void execute_one_inst();
    void reset();

    void debug_get_reg_val();

   private:
    void single_instruction();
    word_t pc;
};

#endif  // CORE_DECL_H_