#ifndef NEMU_H_
#define NEMU_H_

class Nemu
{
   private:
    enum State
    {
        RUNNING,
        STOP,
        END,
        ABORT,
        QUIT
    } state;

   public:
    Nemu();
};

#endif