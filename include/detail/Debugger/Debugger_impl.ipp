#ifndef DEBUGGER_IMPL_HPP_
#define DEBUGGER_IMPL_HPP_

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <print>

#include "Exception/NEMUException.hpp"
#include "Utils/Disasm.h"
#include "Utils/ElfParser.h"
#include "detail/Debugger/Debugger_decl.hpp"
#include "readline/history.h"
#include "readline/readline.h"

struct Rule
{
    std::string regex;
    TOKEN_TYPE token_type;
};

const std::array<Rule, 13> rules = {
    Rule{" +", TK_NOTYPE},                 // spaces
    Rule{"\\+", TK_ADD},                   // plus
    Rule{"-", TK_SUB},                     // sub
    Rule{"\\*", TK_MUL},                   // mul
    Rule{"/", TK_DIV},                     // div
    Rule{"==", TK_EQ},                     // equal
    Rule{"\\(", TK_LEFT_BRACKET},          // left bracket
    Rule{"\\)", TK_RIGHT_BRACKET},         // right bracket
    Rule{"\\$[a-zA-Z0-9]+", TK_REGISTER},  // register
    Rule{"0x[0-9a-fA-F]+", TK_HEX},        // hex
    Rule{"[0-9]+", TK_NUMBER},             // number
    Rule{"&&", TK_AND}                     // and
};
std::vector<regex_t> regexs;

static uint32_t get_precedence(TOKEN_TYPE type)
{
    switch (type)
    {
        case TK_AND:
        case TK_EQ:
            return 0;
        case TK_ADD:
        case TK_SUB:
            return 1;
        case TK_MUL:
        case TK_DIV:
            return 2;
        default:
            return 0x3f3f3f3f;
    }
}

void regex_init(std::vector<regex_t>& regexs)
{
    if (regexs.size() != 0) return;
    for (auto& rule : rules)
    {
        regex_t re;
        int ret = regcomp(&re, rule.regex.c_str(), REG_EXTENDED);
        if (ret != 0)
        {
            char buf[512];
            regerror(ret, &re, buf, sizeof(buf));
            spdlog::error("regcomp: {}", buf);
            regexs.clear();
            assert(false);
        }
        regexs.push_back(re);
    }
}

bool make_token(std::string expr, std::vector<Token>& tokens)
{
    tokens.clear();
    size_t pos = 0;
    while (pos < expr.size())
    {
        bool match = false;
        for (size_t i = 0; i < rules.size(); i++)
        {
            regmatch_t pmatch;
            int ret = regexec(&regexs[i], expr.c_str() + pos, 1, &pmatch, 0);
            if (ret == 0 && pmatch.rm_so == 0)
            {
                Token token;
                token.str = expr.substr(pos, pmatch.rm_eo);
                token.type = rules[i].token_type;
                tokens.push_back(token);
                pos += pmatch.rm_eo;
                match = true;
                break;
            }
        }
        if (!match)
        {
            spdlog::error("No match at position {}, {}", pos, expr.substr(pos));
            return false;
        }
    }
    return true;
}

bool check_parentheses(int p, int q, std::vector<Token>& tokens)
{
    int cnt = 0;
    if (tokens[p].type != TK_LEFT_BRACKET || tokens[q].type != TK_RIGHT_BRACKET)
        return false;
    for (int i = p + 1; i < q; i++)
    {
        if (tokens[i].type == TK_LEFT_BRACKET)
            cnt++;
        else if (tokens[i].type == TK_RIGHT_BRACKET)
            cnt--;
        if (cnt < 0) return false;
    }
    return cnt == 0;
}

bool domain_operator(int p, int q, std::vector<Token>& tokens)
{
    int cnt = 0;
    uint32_t min_priority = 0x3f3f3f3f;
    int op = -1;
    for (int i = p; i <= q; i++)
    {
        if (tokens[i].type == TK_LEFT_BRACKET)
        {
            cnt++;
        }
        else if (tokens[i].type == TK_RIGHT_BRACKET)
        {
            cnt--;
        }
        else if (cnt == 0 && get_precedence(tokens[i].type) <= min_priority &&
                 (!(tokens[i].type == TK_ADD || tokens[i].type == TK_SUB ||
                    tokens[i].type == TK_MUL) ||
                  tokens[i - 1].type == TK_RIGHT_BRACKET ||
                  tokens[i - 1].type == TK_NUMBER ||
                  tokens[i - 1].type == TK_REGISTER ||
                  tokens[i - 1].type == TK_HEX || i == p))
        {
            min_priority = get_precedence(tokens[i].type);
            op = i;
        }
    }
    return op;
}

static inline char* rl_gets()
{
    static char* line_read = NULL;

    if (line_read)
    {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read)
    {
        add_history(line_read);
    }

    return line_read;
}

template <typename T>
Debugger<T>::Debugger(Monitor<T>& monitor, std::filesystem::path elf_file)
    : monitor(monitor),
      commands({
          {"c", "Continue", &Debugger<T>::cmd_c},
          {"info", "Print information: r for register; w for watchpoint",
           &Debugger<T>::cmd_info},
          {"si", "Single instruction", &Debugger<T>::cmd_si},
          {"x", "Examine memory", &Debugger<T>::cmd_x},
          {"p", "Print expression", &Debugger<T>::cmd_p},
          {"q", "Quit", &Debugger<T>::cmd_q},
          {"w", "Set watchpoint", &Debugger<T>::cmd_w},
          {"d", "Delete watchpoint", &Debugger<T>::cmd_d},
          {"help", "Print help", &Debugger<T>::cmd_help},
      })
{
    regex_init(regexs);
    for (int i = 0; i < 32; i++)
    {
        watchpoint_free_list.push_back(i);
    }

    if (std::filesystem::exists(elf_file))
    {
        symbols =
            std::make_unique<SymbolTable>(getFunctionSymbol(elf_file).value());
    }
}

template <typename T>
Debugger<T>::~Debugger()
{
}

template <typename T>
bool Debugger<T>::check_watchpoint()
{
    for (auto wp : watchpoint_used_list)
    {
        bool flag;
        int value = evaluate(watchpoint_pool[wp].expr, flag);
        if (!flag)
        {
            printf("Invalid expression\n");
            return false;
        }
        if (value != watchpoint_pool[wp].value)
        {
            printf("Watchpoint %d: %s\n", wp, watchpoint_pool[wp].expr.c_str());
            printf("Old value: %d\n", watchpoint_pool[wp].value);
            printf("New value: %d\n", value);
            watchpoint_pool[wp].value = value;
            return true;
        }
    }
    return false;
}

template <typename T>
void Debugger<T>::execute(uint64_t step)
{
    try
    {
        while (step--)
        {
            word_t pc = monitor.get_reg_val("pc");
            word_t inst = monitor.mem_read(pc, 4);
            latest_instrution = instruction_buffer.push(
                disassemble(pc, (uint8_t*)&inst, sizeof(typename T::word_t)));
            monitor.execute(1);
#ifdef CHECK_WATCHPOINT
            if (check_watchpoint()) break;
#endif
        }
    }
    catch (program_halt& e)
    {
        spdlog::info("Program halted");
    }
}

template <typename T>
int Debugger<T>::cmd_c()
{
    auto args = strtok(nullptr, " ");
    if (args != nullptr)
    {
        printf("Command 'c' does not accept any arguments\n");
        return 1;
    }
    execute(-1);
    return 0;
}

template <typename T>
int Debugger<T>::cmd_info()
{
    auto args = strtok(nullptr, " ");
    if (args == nullptr)
    {
        printf("Command 'info' requires an argument\n");
        return 1;
    }
    if (strcmp(args, "r") == 0)
    {
        monitor.print_registers();
    }
    else if (strcmp(args, "w") == 0)
    {
        for (auto wp : watchpoint_used_list)
        {
            std::print("Watchpoint {}: {} = {}\n", wp, watchpoint_pool[wp].expr,
                       watchpoint_pool[wp].value);
        }
    }
    else
    {
        printf("Invalid argument for command 'info'\n");
    }
    return 0;
}

template <typename T>
int Debugger<T>::cmd_si()
{
    auto args = strtok(nullptr, " ");
    if (args != nullptr)
    {
        printf("Command 'si' does not accept any arguments\n");
        return 1;
    }
    execute(1);
#ifdef TRACE_INSTRUCTION
    std::print("Current instruction: \n");
    std::print("{}\n", latest_instrution);
#endif
    return 0;
}

template <typename T>
int Debugger<T>::cmd_x()
{
    auto args = strtok(nullptr, " ");
    if (args == nullptr)
    {
        printf("Invalid first argument for command 'x'\n");
        return 1;
    }
    int n = atoi(args);
    args = strtok(nullptr, " ");
    if (args == nullptr)
    {
        printf("Invalid second argument for command 'x'\n");
        return 1;
    }
    Memory::paddr_t address;
    bool flag;
    address = evaluate(args, flag);
    if (!flag)
    {
        printf("Invalid expression\n");
        return 1;
    }
    for (int i = 0; i < n; i++)
    {
        word_t result = monitor.mem_read(address, 4);
        printf("%08x: %08x\n", address, result);
        address += 4;
    }
    return 0;
}
template <typename T>
int Debugger<T>::cmd_p()
{
    bool flag;
    auto args = strtok(nullptr, " ");
    unsigned result = evaluate(args, flag);
    if (!flag)
    {
        printf("Invalid expression\n");
    }
    else
    {
        printf("%u\n", result);
    }
    return 0;
}

template <typename T>
int Debugger<T>::cmd_q()
{
    auto args = strtok(nullptr, " ");
    if (args != nullptr)
    {
        printf("Command 'q' does not accept any arguments\n");
        return 1;
    }
    monitor.quit();
    return -1;
}

template <typename T>
int Debugger<T>::cmd_w()
{
    auto args = strtok(nullptr, " ");
    if (args == nullptr)
    {
        printf("Command 'w' requires an argument\n");
        return 1;
    }

    bool flag;
    auto res = evaluate(args, flag);
    if (!flag)
    {
        printf("Invalid expression\n");
        return 1;
    }

    if (watchpoint_free_list.empty())
    {
        printf("No free watchpoint\n");
        return 1;
    }
    int wp_id = watchpoint_free_list.front();
    watchpoint_free_list.pop_front();
    watchpoint_used_list.push_back(wp_id);
    watchpoint_pool[wp_id].expr = args;
    watchpoint_pool[wp_id].value = res;

    printf("Set watchpoint %d: %s\n", wp_id, args);

    return 0;
}

template <typename T>
int Debugger<T>::cmd_d()
{
    auto args = strtok(nullptr, " ");
    if (args == nullptr)
    {
        printf("Command 'd' requires an argument\n");
        return 1;
    }
    int wp_id = atoi(args);
    if (wp_id < 0 || wp_id >= 32)
    {
        printf("Invalid watchpoint id\n");
        return 1;
    }
    if (watchpoint_used_list.empty())
    {
        printf("No watchpoints to delete\n");
        return 1;
    }

    bool found = false;
    for (auto wp : watchpoint_used_list)
    {
        if (wp == wp_id)
        {
            found = true;
            watchpoint_used_list.remove(wp_id);
            break;
        }
    }
    if (!found)
    {
        printf("Watchpoint %d not found\n", wp_id);
        return 1;
    }
    else
    {
        watchpoint_free_list.push_back(wp_id);
        printf("Delete watchpoint %d\n", wp_id);
    }
    return 0;
}

template <typename T>
int Debugger<T>::cmd_help()
{
    auto args = strtok(nullptr, " ");
    if (args != nullptr)
    {
        printf("Command 'help' does not accept any arguments\n");
        return 1;
    }
    printf("Available commands:\n");
    for (const auto& cmd : commands)
    {
        printf("  %s: %s\n", cmd.cmd, cmd.desc);
    }
    return 0;
}

template <typename T>
int Debugger<T>::cmd_handler(char* cmd)
{
    auto arg = strtok(cmd, " ");
    for (const auto& c : commands)
    {
        if (strcmp(arg, c.cmd) == 0)
        {
            return (this->*c.func)();
        }
    }
    printf("Invalid command\n");
    return 1;
}

template <typename T>
int Debugger<T>::run(bool is_batch_mode)
{
    if (is_batch_mode)
    {
        execute(-1);
    }
    else
        try
        {
            while (true)
            {
                char* line = rl_gets();
                if (line == nullptr)
                {
                    break;
                }
                if (cmd_handler(line) < 0) break;
            }
        }
        catch (program_halt& e)
        {
            spdlog::info("Program halted");
        }
    bool is_bad_status = monitor.is_bad_status();
    if (is_bad_status)
    {
        instruction_buffer.print();
    }
    return is_bad_status;
}

template <typename T>
uint32_t Debugger<T>::eval(int p, int q, std::vector<Token> tokens)
{
    if (p > q)
    {
        spdlog::error("Bad expression");
        return 0;
    }
    else if (p == q)
    {
        uint32_t val = 0;
        if (tokens[p].type == TK_NUMBER)
        {
            sscanf(tokens[p].str.c_str(), "%d", &val);
        }
        else if (tokens[p].type == TK_HEX)
        {
            sscanf(tokens[p].str.c_str(), "0x%x", &val);
        }
        else if (tokens[p].type == TK_REGISTER)
        {
            val = monitor.get_reg_val(tokens[p].str.substr(1));
        }
        else
            assert(false);
        return val;
    }
    else if (check_parentheses(p, q, tokens))
    {
        return eval(p + 1, q - 1, tokens);
    }
    else
    {
        int op = domain_operator(p, q, tokens);
        if (p == op)
        {
            word_t address;
            switch (tokens[op].type)
            {
                case TK_ADD:
                    return eval(p + 1, q, tokens);
                case TK_SUB:
                    return -eval(p + 1, q, tokens);
                case TK_MUL:
                    address = eval(p + 1, q, tokens);
                    return monitor.mem_read(address, 4);
                default:
                    return 0;
            }
        }
        uint32_t val1 = eval(p, op - 1, tokens);
        uint32_t val2 = eval(op + 1, q, tokens);
        switch (tokens[op].type)
        {
            case TK_ADD:
                return val1 + val2;
            case TK_SUB:
                return val1 - val2;
            case TK_MUL:
                return val1 * val2;
            case TK_DIV:
                assert(val2 != 0);
                return val1 / val2;
            case TK_EQ:
                return val1 == val2;
            case TK_AND:
                return val1 && val2;
            default:
                spdlog::error("Bad expression");
                return 0;
        }
    }
}

template <typename T>
uint32_t Debugger<T>::evaluate(std::string expr, bool& success)
{
    std::vector<Token> tokens;
    if (!make_token(expr, tokens))
    {
        success = false;
        return 0;
    }
    success = true;
    return eval(0, tokens.size() - 1, tokens);
}

#endif  // DEBUGGER_IMPL_HPP_