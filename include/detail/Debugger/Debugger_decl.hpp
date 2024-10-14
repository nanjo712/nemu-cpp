#ifndef DEBUGGER_DECL_HPP_
#define DEBUGGER_DECL_HPP_

#include <regex.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "Monitor/Monitor.hpp"
#include "Utils/ElfParser.h"
#include "Utils/RingBuffer.h"

enum TOKEN_TYPE
{
    TK_EQ,
    TK_AND,
    TK_ADD,
    TK_SUB,
    TK_MUL,
    TK_DIV,
    TK_NUMBER,
    TK_LEFT_BRACKET = 256,
    TK_RIGHT_BRACKET,
    TK_REGISTER,
    TK_HEX,
    /* TODO: Add more token types */
    TK_NOTYPE,

};

struct Token
{
    TOKEN_TYPE type;
    std::string str;
};

template <typename T>
class Debugger
{
   public:
    Debugger(Monitor<T>& monitor, std::filesystem::path elf_file = "");
    ~Debugger();

    int run(bool is_batch_mode = false);

   private:
    Monitor<T>& monitor;
    RingBuffer<std::string, 32> instruction_buffer;
    std::string_view latest_instrution;

    struct Command
    {
        const char* cmd;
        const char* desc;
        int (Debugger::*func)();
    };
    std::vector<Command> commands;

    struct WatchPoint
    {
        std::string expr;
        int value;
    };
    std::array<WatchPoint, 32> watchpoint_pool;
    std::list<int> watchpoint_free_list;
    std::list<int> watchpoint_used_list;

    std::unique_ptr<SymbolTable> symbols;

    int cmd_c();
    int cmd_info();
    int cmd_si();
    int cmd_x();
    int cmd_p();
    int cmd_q();
    int cmd_w();
    int cmd_d();
    int cmd_help();

    int cmd_handler(char* cmd);

    bool check_watchpoint();

    void execute(uint64_t step);
    uint32_t eval(int p, int q, std::vector<Token> tokens);
    uint32_t evaluate(std::string expr, bool& success);
};

#endif