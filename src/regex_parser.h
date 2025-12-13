/*
 * regex_parser.h - defines the core interfaces for parsing and converting a regular expression
 * into an NFA. It provides:
 * - Token representation: supports operators and 'CharSet'-based operands.
 * - Preprocessing utilities: `preprocessRegex` tokenizes a raw regex string and handles
 * character classes (e.g., [a-z]), while `insertConcatSymbols` inserts explicit concatenation
 * operators (denoted by `EXPLICIT_CONCAT_OP`) where needed.
 * - InfixToPostfix: converts a tokenized infix regex into postfix notation using
 * the Shunting-yard algorithm with custom ISP/ICP precedence rules.
 * - regexToNFA: constructs an NFA from a postfix token sequence using Thompson’s construction.
 * - Supporting declarations: a global `EXPLICIT_CONCAT_OP` constant, a `RegexSyntaxError`
 * exception type for parse-time errors, and `CharSet` from `nfa.h` for symbol representation.
 */

#pragma once

#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include "nfa.h" 

// ==========================================
// 全局常量声明
// ==========================================
extern const char EXPLICIT_CONCAT_OP;

// ==========================================
// 自定义异常类
// ==========================================
class RegexSyntaxError : public std::runtime_error {
public:
    // 构造函数，接收错误信息
    explicit RegexSyntaxError(const std::string& message) 
        : std::runtime_error(message) {}
        
    // 可选：如果有需要，可以添加记录错误位置的成员
};

// ==========================================
// Token 定义
// ==========================================
struct Token {
    enum Type { OPERATOR, OPERAND } type;
    char opVal;
    CharSet operandVal;

    Token() : type(OPERATOR), opVal(0) {}
    Token(char op) : type(OPERATOR), opVal(op) {}
    Token(CharSet cs) : type(OPERAND), operandVal(cs) {}
    
    bool isOperator() const { return type == OPERATOR; }
    bool isOperand() const { return type == OPERAND; }
    
    // 辅助：获取 Token 的字符串表示，用于错误提示
    std::string toString() const {
        if (isOperator()) return std::string(1, opVal);
        return operandVal.toString();
    }
};

// ==============================
// 函数声明
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
