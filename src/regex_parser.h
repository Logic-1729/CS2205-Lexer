#ifndef REGEX_PARSER_H
#define REGEX_PARSER_H

#include <string>
#include <vector>
#include <variant>
#include "nfa.h" 

// 全局常量：显式连接符（用于内部表示连接操作，避免与用户输入的 '+' 冲突）
extern const char EXPLICIT_CONCAT_OP;

// Token 类型：可以是操作符(char) 或 操作数(CharSet)
struct Token {
    enum Type { OPERATOR, OPERAND } type;
    char opVal;
    CharSet operandVal;

    // 默认构造函数
    Token() : type(OPERATOR), opVal(0) {}
    
    Token(char op) : type(OPERATOR), opVal(op) {}
    Token(CharSet cs) : type(OPERAND), operandVal(cs) {}
    
    bool isOperator() const { return type == OPERATOR; }
    bool isOperand() const { return type == OPERAND; }
};

// ==============================
// 正则表达式预处理与解析
// ==============================

std::vector<Token> preprocessRegex(const std::string& re);
std::vector<Token> insertConcatSymbols(const std::vector<Token>& tokens);

class InfixToPostfix {
public:
    explicit InfixToPostfix(const std::vector<Token>& infix);
    void convert();
    const std::vector<Token>& getPostfix() const;

private:
    std::vector<Token> infix_;
    std::vector<Token> postfix_;
    int getISP(char op);
    int getICP(char op);
};

NFAUnit regexToNFA(const std::vector<Token>& postfix);

#endif // REGEX_PARSER_H