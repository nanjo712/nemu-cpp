#include "Debugger/Debugger.h"

#include <readline/history.h>
#include <readline/readline.h>

#include <cstring>

#include "Debugger/Expr.h"
#include "ISA/ISA_Wrapper.h"
#include "Memory/Memory.h"
#include "Monitor/Monitor.h"

static char* rl_gets()
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

Debugger& Debugger::getDebugger()
{
    static Debugger debugger;
    return debugger;
}

Debugger::Debugger()
    : monitor(Monitor::getMonitor()),
      commands({
          {"c", "Continue", &Debugger::cmd_c},
          {"info", "Print information: r for register; w for watchpoint",
           &Debugger::cmd_info},
          {"si", "Step into", &Debugger::cmd_si},
          {"x", "Examine memory", &Debugger::cmd_x},
          {"p", "Print expression", &Debugger::cmd_p},
          {"q", "Quit", &Debugger::cmd_q},
          {"w", "Set watchpoint", &Debugger::cmd_w},
          {"help", "Print help", &Debugger::cmd_help},
      })
{
}

Debugger::~Debugger() {}

int Debugger::cmd_c(char* args)
{
    if (args != nullptr)
    {
        printf("Command 'c' does not accept any arguments\n");
        return 1;
    }
    while (monitor.execute(1));
    return 0;
}

int Debugger::cmd_info(char* args)
{
    if (args == nullptr)
    {
        printf("Command 'info' requires an argument\n");
        return 1;
    }
    if (strcmp(args, "r") == 0)
    {
        monitor.isa.display_reg();
    }
    else if (strcmp(args, "w") == 0)
    {
        printf("Not implemented yet\n");
    }
    else
    {
        printf("Invalid argument for command 'info'\n");
    }
    return 0;
}

int Debugger::cmd_si(char* args)
{
    if (args == nullptr)
    {
        return !monitor.execute(1);
    }
    else
    {
        printf("Command 'si' does not accept any arguments\n");
        return 1;
    }
}

int Debugger::cmd_x(char* args)
{
    char* arg = strtok(args, " ");
    if (arg == nullptr)
    {
        printf("Invalid argument for command 'x'\n");
        return 1;
    }
    int n = atoi(arg);
    arg = strtok(nullptr, " ");
    if (arg == nullptr)
    {
        printf("Invalid argument for command 'x'\n");
        return 1;
    }
    Memory::paddr_t address;
    bool flag;
    address = Expression().evaluate(arg, flag);
    if (!flag)
    {
        printf("Invalid expression\n");
        return 1;
    }
    for (int i = 0; i < n; i++)
    {
        word_t result = monitor.mem.read(address, 4);
        printf("%08x: %08x\n", address, result);
        address += 4;
    }
    return 0;
}

int Debugger::cmd_p(char* args)
{
    bool flag;
    unsigned result = Expression().evaluate(args, flag);
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

int Debugger::cmd_q(char* args)
{
    if (args != nullptr)
    {
        printf("Command 'q' does not accept any arguments\n");
        return 1;
    }
    monitor.quit();
    return -1;
}

int Debugger::cmd_w(char* args)
{
    printf("Not implemented yet\n");
    return 0;
}

int Debugger::cmd_help(char* args)
{
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

int Debugger::cmd_handler(char* cmd)
{
    char* args = strtok(nullptr, " ");
    for (const auto& c : commands)
    {
        if (strcmp(cmd, c.cmd) == 0)
        {
            return (this->*c.func)(args);
        }
    }
    printf("Invalid command\n");
    return 1;
}

int Debugger::run()
{
    while (true)
    {
        char* line = rl_gets();
        if (line == nullptr)
        {
            break;
        }
        char* cmd = strtok(line, " ");
        if (cmd == nullptr)
        {
            continue;
        }
        if (cmd_handler(cmd) < 0) break;
    }
    return 0;
}