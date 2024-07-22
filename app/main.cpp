#include <iostream>

#include "Monitor/Monitor.h"

class Nemu
{
   public:
    Nemu()
    {
        Monitor monitor;
        monitor.execute(-1);
    }
    ~Nemu() { std::cout << "Nemu Destructor" << std::endl; }
};

int main()
{
    Nemu nemu;
    return 0;
}