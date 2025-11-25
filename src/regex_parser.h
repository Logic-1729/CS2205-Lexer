#ifndef REGEX_PARSER_H
#define REGEX_PARSER_H

#include <string>
#include <vector>
#include "nfa.h" 

// ==============================
// 正则表达式预处理与解析
// ==============================

std::vector<std::string> preprocessRegex(const std::string& re);
std::vector<std::string> insertConcatSymbols(const std::vector<std::string>& tokens);

class InfixToPostfix {
public:
    explicit InfixToPostfix(const std::vector<std::string>& infix);
    void convert();
    const std::vector<std::string>& getPostfix() const;

private:
    std::vector<std::string> infix_;
    std::vector<std::string> postfix_;
    bool isLetter(const std::string& token);
    int getISP(char op);
    int getICP(char op);
};

// 从后缀表达式生成 NFA
NFAUnit regexToNFA(const std::vector<std::string>& postfix);

// 辅助函数
bool isLetter(const std::string& s);

#endif // REGEX_PARSER_H