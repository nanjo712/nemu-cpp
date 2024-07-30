#include "ISA/riscv32/Reset.h"

#include <spdlog/spdlog.h>

#include <fstream>

#include "ISA/riscv32/Register.h"

Reset_Handler::Reset_Handler(Register& reg) : reg(reg) {};

Reset_Handler::~Reset_Handler() {};

void Reset_Handler::set_img(const std::string& img_file)
{
    if (img_file == "null")
    {
        img = std::vector<word_t>(built_in_img.begin(), built_in_img.end());
    }
    else
    {
        // read img_file in binary mode
        std::ifstream file(img_file, std::ios::binary);
        if (!file.is_open())
        {
            spdlog::error("Cannot open file: {}", img_file);
            exit(1);
        }
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        img.resize(size / 4);
        file.read(reinterpret_cast<char*>(img.data()), size);
        file.close();
    }
}

const std::vector<word_t>& Reset_Handler::get_img() { return img; }

void Reset_Handler::reset()
{
    reg.setPC(reset_vector);
    reg.write(0, 0);
}
