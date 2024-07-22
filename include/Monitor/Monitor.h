#ifndef MONITOR_H_
#define MONITOR_H_

#include <chrono>
#include <cstdint>
#include <memory>

#include "Utils/Utils.h"

class CPU;
class Memory;
class ISA_Wrapper;
class Monitor
{
   public:
    Monitor();
    ~Monitor();
    void execute(uint64_t n);
    void statistics();

   private:
    enum State
    {
        RUNNING,
        STOP,
        END,
        ABORT,
        QUIT
    } state;
    word_t halt_pc;
    word_t halt_ret;
    std::unique_ptr<CPU> cpu;
    std::unique_ptr<Memory> mem;
    std::unique_ptr<ISA_Wrapper> isa;
    std::chrono::nanoseconds timer;
    uint64_t inst_count;

    void trap_handler(State s, word_t pc, word_t ret);
    void invaild_inst_handler(word_t pc);
    void ebreak_handler(word_t pc);
};

#endif  // MONITOR_H_