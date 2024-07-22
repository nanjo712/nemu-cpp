#ifndef CPU_H_
#define CPU_H_

#include <cstdint>

class ISA_Wrapper;
class CPU
{
   public:
    CPU(ISA_Wrapper& isa);
    ~CPU();
    void execute(uint64_t n);

   private:
    ISA_Wrapper& isa;
};

#endif