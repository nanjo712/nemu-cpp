#include "ISA/ISA_Wrapper.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <iostream>

#include "Memory/Memory.h"
#include "Utils/Disasm.h"

std::string ISA_Wrapper::img_file = "null";

ISA_Wrapper& ISA_Wrapper::getISA()
{
    static ISA_Wrapper isa;
    return isa;
};

ISA_Wrapper::ISA_Wrapper()
    : mem(Memory::getMemory()), reg(), executor(reg), reset_handler(reg)
{
    if (img_file != "null")
    {
        spdlog::info("Using image file: {}", img_file);
    }
    else
    {
        spdlog::info("Using built-in image");
    }
    reset_handler.set_img(img_file);
    reset_handler.reset();
    init_disasm(
        "riscv32"
        "-pc-linux-gnu");
    load_img();
};

ISA_Wrapper::~ISA_Wrapper() {};

void ISA_Wrapper::execute_one_inst()
{
    word_t pc = reg.getPC();
    word_t inst = mem.read(pc, 4);
    executor.execute(inst);
};

void ISA_Wrapper::display_reg()
{
    for (int i = 0; i < 32; i++)
    {
        std::cout << fmt::format("{0}: 0x{1:x}\n", Register::regNames[i],
                                 reg.read(i));
    }
    std::cout << fmt::format("PC: 0x{0:x}\n", reg.getPC());
};

void ISA_Wrapper::load_img()
{
    auto& img = reset_handler.get_img();
    for (auto i = 0u; i < img.size(); i++)
    {
        mem.write(MEMORY_BASE + i * sizeof(word_t), img[i], sizeof(word_t));
    }
};

word_t ISA_Wrapper::get_reg_val(const std::string_view reg_name)
{
    if (reg_name == "pc")
        return reg.getPC();
    else
        return reg.read(reg.getRegIndex(std::move(reg_name)));
};