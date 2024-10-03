#ifndef MONITOR_DECL_HPP_
#define MONITOR_DECL_HPP_

#include <chrono>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include "Core/Core.hpp"
#include "Memory/Memory.h"

template <typename T>
class Monitor
{
    static_assert(std::is_base_of<Core<T>, T>::value,
                  "T must be derived from Core<T>");
    using word_t = typename T::word_t;
    using sword_t = typename T::sword_t;

   public:
    Monitor(Core<T> &core, Memory &memory);
    ~Monitor();

    void execute(uint64_t n);
    void quit();
    void print_registers();
    auto get_reg_val(std::string_view reg_name);
    auto mem_read(word_t addr, size_t len);

    void invalid_inst_handler(word_t pc);
    void ebreak_handler(word_t pc);

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

    Core<T> &core;
    Memory &memory;
    word_t halt_pc;
    word_t halt_ret;
    std::chrono::nanoseconds timer;
    uint64_t inst_count;

    void statistics();
};

#endif  // MONITOR_DECL_HPP_