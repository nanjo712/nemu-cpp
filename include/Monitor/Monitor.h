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
    Monitor(const Monitor&) = delete;
    Monitor& operator=(const Monitor&) = delete;
    Monitor(Monitor&&) = delete;
    Monitor& operator=(Monitor&&) = delete;

    ~Monitor();

    static Monitor& getMonitor();
    void execute(uint64_t n);
    void statistics();

    void invalid_inst_handler(word_t pc);
    void ebreak_handler(word_t pc);

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
    Memory& mem;
    ISA_Wrapper& isa;
    std::chrono::nanoseconds timer;
    uint64_t inst_count;

    Monitor();

    void trap_handler(State s, word_t pc, word_t ret);
};

#endif  // MONITOR_H_