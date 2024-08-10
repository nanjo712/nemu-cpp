#ifndef ELF_PARSER_H_
#define ELF_PARSER_H_

#include <optional>
#include <string>
#include <vector>

#include "Utils.h"

struct SymbolInfo
{
    std::string name;
    word_t addr;
    word_t size;
};

using SymbolTable = std::vector<SymbolInfo>;

std::optional<SymbolTable> getFunctionSymbol(const std::string& file_path);

#endif  // ELF_PARSER_H_