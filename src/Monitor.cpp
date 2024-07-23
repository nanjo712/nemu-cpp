#include "Monitor/Monitor.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <iostream>

#include "ISA/ISA_Wrapper.h"
#include "Memory/Memory.h"
#include "Monitor/Monitor.h"

Monitor& Monitor::getMonitor()
{
    static Monitor monitor;
    return monitor;
}

Monitor::Monitor() : mem(Memory::getMemory()), isa(ISA_Wrapper::getISA())
{
    state = State::STOP;
    inst_count = 0;
    timer = std::chrono::nanoseconds(0);
}

Monitor::~Monitor() {}

void Monitor::trap_handler(State s, word_t pc, word_t ret)
{
    state = s;
    halt_pc = pc;
    halt_ret = ret;
}

void Monitor::invalid_inst_handler(word_t pc)
{
    std::cout << fmt::format("Invalid instruction at {0:x}\n", pc);
    trap_handler(State::ABORT, pc, -1);
}

void Monitor::ebreak_handler(word_t pc)
{
    std::cout << fmt::format("Trigger ebreak at {0:x}\n", pc);
    trap_handler(State::END, pc, isa.reg.read(10));  // $10 is a0
}

void Monitor::statistics()
{
    spdlog::info("Execution time: {} ns", timer.count());
    // std::cout << "Instruction count: " << inst_count << std::endl;
    spdlog::info("Instruction count: {}", inst_count);
    if (timer.count() > 0)
        spdlog::info("Instruction per s: {}", inst_count * 1e9 / timer.count());
    else
        spdlog::info("Simulation time is too short to calculate IPS.");
}

void Monitor::execute(uint64_t n)
{
    if (state == State::END || state == State::ABORT)
    {
        std::cout << "Program execution has ended. To restart the program, "
                     "exit Monitor and run again.\n";
        return;
    }
    else
    {
        state = State::RUNNING;
    }

    auto start = std::chrono::steady_clock::now();

    for (uint64_t i = 0; i < n; i++)
    {
        isa.execute_one_inst();
        inst_count++;
        if (state != State::RUNNING)
        {
            break;
        }
        // TODO: Update Device
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
        spdlog::info("Halt PC = {0:x}, Halt Return Value = {1:x}", halt_pc,
                     halt_ret);
        spdlog::info("Hit {} Trap", state == State::END ? "Good" : "Bad");
    }
    else if (state == State::QUIT)
    {
        statistics();
    }
}