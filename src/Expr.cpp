#include "Debugger/Expr.h"

#include <regex.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdint>
#include <string>

#include "ISA/ISA_Wrapper.h"
#include "Memory/Memory.h"
#include "Utils/Utils.h"

const std::array<Expression::Rule, 13> Expression::rules = {
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

uint32_t Expression::get_precedence(TOKEN_TYPE type)
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

Expression::Expression() : monitor(Monitor::getMonitor())
{
    if (regexs.size() != 0) return;
    for (auto &rule : rules)
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

Expression::~Expression() {}

uint32_t Expression::evaluate(std::string expr, bool &success)
{
    if (!make_token(expr))
    {
        success = false;
        return 0;
    }

    success = true;
    uint32_t ret = eval(0, tokens.size() - 1);
    return ret;
}

bool Expression::make_token(std::string expr)
{
    tokens.clear();
    long unsigned int pos = 0;
    while (pos < expr.size())
    {
        bool match = false;
        for (auto i = 0ul; i < rules.size(); i++)
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

bool Expression::check_parentheses(int p, int q)
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

int Expression::dominant_operator(int p, int q)
{
    int i, op = p, cnt = 0;
    uint32_t min_priority = 0x3f3f3f3f;
    for (i = p; i <= q; i++)
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
                 // 检查是否是一元运算符，如果是则跳过
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

uint32_t Expression::eval(int p, int q)
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
            val = monitor.isa.get_reg_val(tokens[p].str.substr(1));
        }
        else
            assert(false);
        return val;
    }
    else if (check_parentheses(p, q))
    {
        return eval(p + 1, q - 1);
    }
    else
    {
        int op = dominant_operator(p, q);
        if (p == op)
        {
            word_t address;
            switch (tokens[op].type)
            {
                case TK_ADD:
                    return eval(p + 1, q);
                case TK_SUB:
                    return -eval(p + 1, q);
                case TK_MUL:
                    address = eval(p + 1, q);
                    return monitor.mem.read(address, 4);
                default:
                    return 0;
            }
        }
        uint32_t val1 = eval(p, op - 1);
        uint32_t val2 = eval(op + 1, q);
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
