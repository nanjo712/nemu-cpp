#ifndef DEBUGGER_H_
#define DEBUGGER_H_

class ISA_Wrapper;
class Memory;
class Debugger
{
   public:
    Debugger();
    ~Debugger();

   private:
    Memory& mem;
    ISA_Wrapper& isa;
};

#endif