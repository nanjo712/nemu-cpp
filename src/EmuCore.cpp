#include "Core/EmuCore.h"

EmuCore::EmuCore(Memory& mem) : mem(mem) {}

EmuCore::~EmuCore() {}

void EmuCore::reset_impl() {}

void EmuCore::single_instruction_impl() {}
