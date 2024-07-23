#ifndef EXPR_H_
#define EXPR_H_

#include <regex.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

class Expression
{
   public:
    Expression();
    ~Expression();
    uint32_t evaluate(std::string expr);

   private:
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
    struct Rule
    {
        std::string regex;
        TOKEN_TYPE token_type;
    };
    static const std::array<Rule, 13> rules;
    static std::vector<regex_t> regex;
    static uint32_t get_precedence(TOKEN_TYPE type);

    struct Token
    {
        TOKEN_TYPE type;
        std::string str;
    };
    std::vector<Token> tokens;

    bool make_token(std::string expr);
    bool check_parentheses(int p, int q);
    int dominant_operator(int p, int q);
    uint32_t eval(int p, int q);
};

#endif