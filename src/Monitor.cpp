#include "Monitor/Monitor.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <iostream>

#include "ISA/ISA_Wrapper.h"
#include "Memory/Memory.h"
#include "Monitor/Monitor.h"
#include "Utils/Disasm.h"
#include "Utils/Elf_Parser.h"
#include "Utils/Ring_Buffer.h"
#include "Utils/Utils.h"

std::string Monitor::elf_file = "";

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
#ifdef TRACE_FUNCTION
    sym_table = getFunctionSymbol(elf_file);
    if (!sym_table.has_value())
    {
        spdlog::warn("Failed to load symbol table.");
    }
#endif
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
    word_t inst = mem.instructionFetch(pc);
    std::cout << fmt::format("Instruction: \n BIN:{0:b}\n HEX:{0:x}\n", inst);
    trap_handler(State::ABORT, pc, -1);
}

void Monitor::ebreak_handler(word_t pc)
{
    std::cout << fmt::format("Trigger ebreak at {0:x}\n", pc);
    auto next_state = isa.reg.read(10) == 0 ? State::END : State::ABORT;
    trap_handler(next_state, pc, isa.reg.read(10));  // $10 is a0
}

void Monitor::statistics()
{
    spdlog::info("Execution time: {} ns", timer.count());
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
    else if (state == State::QUIT)
    {
        return;
    }
    else
    {
        state = State::RUNNING;
    }

    auto start = std::chrono::steady_clock::now();

    for (uint64_t i = 0; i < n; i++)
    {
#ifdef TRACE_INSTRUCTION
        auto pc = isa.get_reg_val("pc");
        auto inst = mem.instructionFetch(pc);
        auto disas_inst = disassemble(pc, (uint8_t*)&inst, sizeof(word_t));
        std::cout << disas_inst << std::endl;
        inst_buffer.push(InstInfo{pc, inst});
#endif
        isa.execute_one_inst();
        inst_count++;
        if (state != State::RUNNING)
        {
            break;
        }
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
        if (state == State::ABORT)
        {
            spdlog::error("Program aborted.");
#ifdef TRACE_INSTRUCTION
            while (!inst_buffer.empty())
            {
                auto inst = inst_buffer.pop();
                auto disas_inst =
                    disassemble(inst.pc, (uint8_t*)&inst.inst, sizeof(word_t));
                spdlog::info(disas_inst);
            }
#endif
#ifdef TRACE_FUNCTION
            for (auto& cr : call_record)
            {
                spdlog::info(cr);
            }
#endif
        }
        else if (state == State::END)
        {
            spdlog::info("Program ended.");
        }
    }
    else if (state == State::QUIT)
    {
        statistics();
    }
}

void Monitor::quit()
{
    if (state != State::ABORT) state = State::QUIT;
}

void Monitor::stop() { state = State::STOP; }

bool Monitor::is_bad_status()
{
    if (state == State::ABORT)
    {
        spdlog::error("NEMU exited with bad status.");
        return true;
    }
    return false;
}

const std::optional<SymbolTable>& Monitor::get_sym_table() { return sym_table; }