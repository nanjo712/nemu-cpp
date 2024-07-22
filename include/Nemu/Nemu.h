#ifndef NEMU_H_
#define NEMU_H_

#include <memory>

class CPU;
class Memory;
class ISA_Wrapper;
class Nemu
{
   public:
    Nemu();
    ~Nemu();

   private:
    enum State
    {
        RUNNING,
        STOP,
        END,
        ABORT,
        QUIT
    } state;
    std::unique_ptr<CPU> cpu;
    std::unique_ptr<Memory> mem;
    std::unique_ptr<ISA_Wrapper> isa;
};

#endif