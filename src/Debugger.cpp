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

int Debugger::cmd_c()
{
    auto args = strtok(nullptr, " ");
    if (args != nullptr)
    {
        printf("Command 'c' does not accept any arguments\n");
        return 1;
    }
    while (monitor.execute(1));
    return 0;
}

int Debugger::cmd_info()
{
    auto args = strtok(nullptr, " ");
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

int Debugger::cmd_si()
{
    auto args = strtok(nullptr, " ");
    if (args != nullptr)
    {
        printf("Command 'si' does not accept any arguments\n");
        return 1;
    }
    return !monitor.execute(1);
}

int Debugger::cmd_x()
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
    address = Expression().evaluate(args, flag);
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

int Debugger::cmd_p()
{
    bool flag;
    auto args = strtok(nullptr, " ");
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

int Debugger::cmd_q()
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

int Debugger::cmd_w()
{
    printf("Not implemented yet\n");
    return 0;
}

int Debugger::cmd_help()
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

int Debugger::cmd_handler(char* cmd)
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

int Debugger::run()
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
    return 0;
}