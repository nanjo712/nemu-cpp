#include "Monitor/Monitor.h"

#include <fmt/core.h>

#include <chrono>
#include <iostream>
#include <memory>

#include "CPU/CPU.h"
#include "ISA/ISA_Wrapper.h"
#include "Memory/Memory.h"
#include "Monitor/Monitor.h"

Monitor::Monitor()
{
    state = State::STOP;
    mem = std::make_unique<Memory>();
    isa = std::make_unique<ISA_Wrapper>(
        *mem, [this](word_t pc) { ebreak_handler(pc); },
        [this](word_t pc) { invaild_inst_handler(pc); });
    cpu = std::make_unique<CPU>(*isa);
}

Monitor::~Monitor() {}

void Monitor::trap_handler(State s, word_t pc, word_t ret)
{
    state = s;
    halt_pc = pc;
    halt_ret = ret;
}

void Monitor::invaild_inst_handler(word_t pc)
{
    std::cout << fmt::format("Invalid instruction at {0:x}\n", pc);
    trap_handler(State::ABORT, pc, -1);
}

void Monitor::ebreak_handler(word_t pc)
{
    std::cout << fmt::format("Ebreak at {0:x}\n", pc);
    trap_handler(State::END, pc, isa->getReg().read(10));
}

void Monitor::statistics()
{
    std::cout << "Execution time: " << timer.count() << " ms\n";
    std::cout << "Instruction count: " << inst_count << std::endl;
    if (timer.count() > 0)
        std::cout << "Inst per ms: " << (double)inst_count / timer.count()
                  << std::endl;
    else
        std::cout << "Simulation time is too short to calculate inst per ms\n";
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

    // cpu->execute(n);
    for (uint64_t i = 0; i < n; i++)
    {
        isa->execute_one_inst();
        inst_count++;
        if (state != State::RUNNING)
        {
            break;
        }
        // TODO: Update Device
    }

    auto end = std::chrono::steady_clock::now();

    timer = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (state == State::RUNNING)
    {
        state = State::STOP;
    }
    else if (state == State::END || state == State::ABORT)
    {
        statistics();
    }
    else if (state == State::QUIT)
    {
        statistics();
    }
}