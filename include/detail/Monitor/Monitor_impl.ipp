#ifndef MONITOR_IMPL_IPP_
#define MONITOR_IMPL_IPP_

#include <spdlog/spdlog.h>

#include <iostream>
#include <string_view>

#include "Exception/NEMUException.hpp"
#include "Monitor_decl.hpp"

template <typename T>
Monitor<T>::Monitor(Core<T> &core, Memory &memory) : core(core), memory(memory)
{
    state = State::STOP;
    inst_count = 0;
    timer = std::chrono::nanoseconds(0);
}

template <typename T>
Monitor<T>::~Monitor()
{
}

template <typename T>
void Monitor<T>::invalid_inst_handler(word_t pc)
{
    word_t inst [[maybe_unused]] = memory.read(pc, sizeof(word_t));
    spdlog::error("Invalid instruction at PC = {0:x}", pc);
    spdlog::error("Instrution: \n BIN:{0:b}\n HEX:{0:x}", inst);
    halt_pc = pc;
    state = State::END;
}

template <typename T>
void Monitor<T>::ebreak_handler(word_t pc)
{
    halt_pc = pc;
    halt_ret = core.debug_get_reg_val(10);
    spdlog::info("EBREAK at PC = {0:x}", pc);
    state = State::END;
}

template <typename T>
void Monitor<T>::statistics()
{
    spdlog::info("Execution time: {} ns", timer.count());
    spdlog::info("Instruction count: {}", inst_count);
    if (timer.count() > 0)
        spdlog::info("Instruction per s: {}", inst_count * 1e9 / timer.count());
    else
        spdlog::info("Simulation time is too short to calculate IPS.");
    spdlog::info("Halt PC = {0:x}, Halt Return Value = {1:x}", halt_pc,
                 halt_ret);
}

template <typename T>
void Monitor<T>::execute(uint64_t n)
{
    if (state == State::STOP)
    {
        state = State::RUNNING;
    }
    else
    {
        throw program_halt();
    }

    auto start = std::chrono::steady_clock::now();

    for (uint64_t i = 0; i < n; i++)
    {
        try
        {
            core.execute_one_inst();
        }
        catch (invalid_instruction &e)
        {
            invalid_inst_handler(core.debug_get_pc());
            break;
        }
        catch (ebreak_exception &e)
        {
            ebreak_handler(core.debug_get_pc());
            break;
        }
        inst_count++;
    }

    auto end = std::chrono::steady_clock::now();

    timer = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    if (state == State::RUNNING)
    {
        state = State::STOP;
    }
    else if (state == State::END || state == State::ABORT)
    {
        statistics();
        throw program_halt();
    }
}

template <typename T>
void Monitor<T>::quit()
{
    state = State::QUIT;
}

template <typename T>
void Monitor<T>::print_registers()
{
    for (int i = 0; i < 32; i++)
    {
        std::cout << spdlog::fmt_lib::format("x{0}: {1:x}", i,
                                             core.debug_get_reg_val(i));
    }
}

template <typename T>
auto Monitor<T>::get_reg_val(std::string_view reg_name)
{
    auto reg_num = core.debug_get_reg_index(reg_name);
    return core.debug_get_reg_val(reg_num);
}

template <typename T>
auto Monitor<T>::mem_read(word_t addr, size_t len)
{
    return memory.read(addr, len);
}

#endif  // MONITOR_IMPL_IPP_