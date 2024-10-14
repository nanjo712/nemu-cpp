#include <elf.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "Utils/ElfParser.h"

std::optional<SymbolTable> getFunctionSymbol(const std::string &file_path)
{
    std::fstream elf_file(file_path, std::ios::in | std::ios::binary);
    if (!elf_file.is_open())
    {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return std::nullopt;
    }

    char e_ident[EI_NIDENT];
    elf_file.read(e_ident, EI_NIDENT);
    if (std::strncmp(e_ident, ELFMAG, SELFMAG) != 0)
    {
        std::cerr << "Not an ELF file: " << file_path << std::endl;
        return std::nullopt;
    }

    SymbolTable symbols;
    if (e_ident[EI_CLASS] == ELFCLASS32)
    {
        Elf32_Ehdr header;
        elf_file.seekg(0);
        elf_file.read(reinterpret_cast<char *>(&header), sizeof(header));

        Elf32_Shdr section_header;
        for (int i = 0; i < header.e_shnum; i++)
        {
            elf_file.seekg(header.e_shoff + i * header.e_shentsize);
            elf_file.read(reinterpret_cast<char *>(&section_header),
                          sizeof(section_header));
            if (section_header.sh_type == SHT_SYMTAB)
            {
                Elf32_Shdr strtab_header;
                elf_file.seekg(header.e_shoff +
                               section_header.sh_link * header.e_shentsize);
                elf_file.read(reinterpret_cast<char *>(&strtab_header),
                              sizeof(strtab_header));

                elf_file.seekg(strtab_header.sh_offset);
                std::unique_ptr<char[]> strtab(new char[strtab_header.sh_size]);
                elf_file.read(strtab.get(), strtab_header.sh_size);

                Elf32_Sym symbol;
                for (auto j = 0u;
                     j < section_header.sh_size / section_header.sh_entsize;
                     j++)
                {
                    elf_file.seekg(section_header.sh_offset +
                                   j * section_header.sh_entsize);
                    elf_file.read(reinterpret_cast<char *>(&symbol),
                                  sizeof(symbol));
                    if (ELF32_ST_TYPE(symbol.st_info) == STT_FUNC)
                    {
                        SymbolInfo info;
                        info.name = &strtab[symbol.st_name];
                        info.addr = symbol.st_value;
                        info.size = symbol.st_size;
                        symbols.push_back(info);
                    }
                }
            }
        }
    }
    else if (e_ident[EI_CLASS] == ELFCLASS64)
    {
        Elf64_Ehdr header;
        elf_file.seekg(0);
        elf_file.read(reinterpret_cast<char *>(&header), sizeof(header));

        Elf64_Shdr section_header;
        for (int i = 0; i < header.e_shnum; i++)
        {
            elf_file.seekg(header.e_shoff + i * header.e_shentsize);
            elf_file.read(reinterpret_cast<char *>(&section_header),
                          sizeof(section_header));
            if (section_header.sh_type == SHT_SYMTAB)
            {
                Elf64_Shdr strtab_header;
                elf_file.seekg(header.e_shoff +
                               section_header.sh_link * header.e_shentsize);
                elf_file.read(reinterpret_cast<char *>(&strtab_header),
                              sizeof(strtab_header));

                elf_file.seekg(strtab_header.sh_offset);
                std::unique_ptr<char[]> strtab(new char[strtab_header.sh_size]);
                elf_file.read(strtab.get(), strtab_header.sh_size);

                Elf64_Sym symbol;
                for (auto j = 0u;
                     j < section_header.sh_size / section_header.sh_entsize;
                     j++)
                {
                    elf_file.seekg(section_header.sh_offset +
                                   j * section_header.sh_entsize);
                    elf_file.read(reinterpret_cast<char *>(&symbol),
                                  sizeof(symbol));
                    if (ELF64_ST_TYPE(symbol.st_info) == STT_FUNC)
                    {
                        SymbolInfo info;
                        info.name = &strtab[symbol.st_name];
                        info.addr = symbol.st_value;
                        info.size = symbol.st_size;
                        symbols.push_back(info);
                    }
                }
            }
        }
    }
    else
    {
        std::cerr << "Unknown ELF class: " << e_ident[EI_CLASS] << std::endl;
        return std::nullopt;
    }

    return symbols;
}