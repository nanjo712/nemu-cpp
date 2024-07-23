#include <spdlog/spdlog.h>

#include <iostream>

#include "Debugger/Debugger.h"

class Nemu
{
   public:
    Nemu()
    {
        spdlog::info("Build time: {}, {}", __TIME__, __DATE__);
        Debugger& debugger = Debugger::getDebugger();
        spdlog::info("Welcome to NEMU!");
        spdlog::info("For help, type \"help\"");
        debugger.run();
    }
    ~Nemu() { std::cout << "Nemu Destructor" << std::endl; }
};

int main()
{
    Nemu nemu;
    return 0;
}