#include "ISA/riscv32/Reset.h"

#include "ISA/riscv32/Register.h"

Reset_Handler::Reset_Handler(Register& reg) : reg(reg) {};

Reset_Handler::~Reset_Handler() {};

const std::array<word_t, 5>& Reset_Handler::get_img() { return img; }

void Reset_Handler::reset()
{
    reg.setPC(reset_vector);
    reg.write(0, 0);
}
