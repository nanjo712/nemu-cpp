#ifndef MONITOR_H_
#define MONITOR_H_

#include <chrono>
#include <cstdint>
#include <optional>

#include "Utils/Elf_Parser.h"
#include "Utils/Ring_Buffer.h"
#include "Utils/Utils.h"

class CPU;
class Memory;
class ISA_Wrapper;
class Debugger;
class Monitor
{
   public:
    static std::string elf_file;
    Memory& mem;
    ISA_Wrapper& isa;

    Monitor(const Monitor&) = delete;
    Monitor& operator=(const Monitor&) = delete;
    Monitor(Monitor&&) = delete;
    Monitor& operator=(Monitor&&) = delete;

    ~Monitor();

    static Monitor& getMonitor();
    void execute(uint64_t n);

    void invalid_inst_handler(word_t pc);
    void ebreak_handler(word_t pc);

    const std::optional<SymbolTable>& get_sym_table();
    std::vector<std::string> call_record;

    bool is_bad_status();

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
    std::chrono::nanoseconds timer;
    uint64_t inst_count;
    struct InstInfo
    {
        word_t pc;
        word_t inst;
    };
    RingBuffer<InstInfo, 16> inst_buffer;
    std::optional<SymbolTable> sym_table;

    Monitor();

    void statistics();
    void trap_handler(State s, word_t pc, word_t ret);
    void stop();
    void quit();

    friend class Debugger;
};

#endif  // MONITOR_H_