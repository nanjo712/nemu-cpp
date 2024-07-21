#ifndef CPU_H_
#define CPU_H_

#include <cstdint>

class CPU
{
   public:
    CPU();
    ~CPU();

    void execute(uint64_t n);

   private:
};

#endif