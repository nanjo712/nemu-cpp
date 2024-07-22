#include "Nemu/Nemu.h"

#include <memory>

#include "CPU/CPU.h"
#include "ISA/ISA_Wrapper.h"
#include "Memory/Memory.h"

Nemu::Nemu()
{
    state = State::STOP;
    mem = std::make_unique<Memory>();
    isa = std::make_unique<ISA_Wrapper>(*mem);
    cpu = std::make_unique<CPU>(*isa);
}

Nemu::~Nemu() {}
