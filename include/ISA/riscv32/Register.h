#ifndef REGISTER_H_
#define REGISTER_H_

#include <string>
#include <string_view>

#include "Utils/Utils.h"

class Register
{
   public:
    static const std::string regNames[32];
    Register();
    ~Register();

    int getRegIndex(const std::string_view regName);
    void write(int reg, word_t data);
    word_t read(int reg);
    void setPC(word_t data);
    word_t getPC();

   private:
    word_t registers[32];
    word_t pc;
};

#endif  // REGISTER_H_