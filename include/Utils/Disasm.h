#ifndef DISASM_H_
#define DISASM_H_

#include <cstdint>
#include <string>

void init_disasm(const char *triple);
std::string disassemble(uint64_t pc, uint8_t *code, int nbyte);

#endif