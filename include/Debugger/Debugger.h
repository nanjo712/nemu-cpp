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
        int (Debugger::*func)(char*);
    };
    std::vector<Command> commands;

    Debugger();
    int cmd_c(char* args);
    int cmd_info(char* arg);
    int cmd_si(char* args);
    int cmd_x(char* args);
    int cmd_p(char* args);
    int cmd_q(char* args);
    int cmd_w(char* args);
    int cmd_help(char* args);

    int cmd_handler(char* cmd);
};

#endif