#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <cstdio>
#include <vector>

class ISA_Wrapper;
class Memory;
class Monitor;
class Debugger
{
   public:
    Debugger(const Debugger&) = delete;
    Debugger& operator=(const Debugger&) = delete;
    Debugger(Debugger&&) = delete;
    Debugger& operator=(Debugger&&) = delete;

    static Debugger& getDebugger();
    ~Debugger();

    int run();

   private:
    Monitor& monitor;

    struct Command
    {
        const char* cmd;
        const char* desc;
        int (Debugger::*func)();
    };
    std::vector<Command> commands;

    Debugger();
    int cmd_c();
    int cmd_info();
    int cmd_si();
    int cmd_x();
    int cmd_p();
    int cmd_q();
    int cmd_w();
    int cmd_help();

    int cmd_handler(char* cmd);
};

#endif