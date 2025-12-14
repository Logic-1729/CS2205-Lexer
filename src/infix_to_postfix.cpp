/*
 * infix_to_postfix.cpp - implements the Shunting-yard algorithm to convert a tokenized infix regular expression
 * into postfix notation (Reverse Polish Notation), which is suitable for subsequent NFA construction.
 * It features:
 * - Supports the following operators: '|', '*', '?', '+', and an explicit concatenation operator
 * (EXPLICIT_CONCAT_OP, typically '&').
 * - Uses distinct In-Stack Priority (ISP) and In-Coming Priority (ICP) tables to correctly
 * handle operator precedence and associativity.
 * - Operator precedence (from highest to lowest): '*', '?', '+' > explicit concatenation > '|'.
 * - Parentheses '(' and ')' are handled according to standard shunting-yard rules.
 * - A sentinel token '#' is appended to both input and operator stack to simplify termination logic.
 * - Syntax errors (e.g., unbalanced parentheses, invalid operator sequences) are detected
 * and reported via `RegexSyntaxError` exceptions.
 */
#include "regex_parser.h"
#include <map>
#include <stack>
#include <stdexcept>

InfixToPostfix::InfixToPostfix(const std::vector<Token>& infix)
    : infix_(infix) {}

int InfixToPostfix::getISP(char op) {
    // 栈内优先级 (In-Stack Priority)
    // 修正后：* > 连接 > |
    static const std::map<char, int> isp = {
        {'|', 3},  // 优先级最低 (原为5)
        {'*', 7}, {'?', 7}, 
        {'(', 1}, {')', 8}, {'#', 0}
    };
    
    // 显式连接符优先级需高于 | 但低于 *
    if (op == EXPLICIT_CONCAT_OP) return 5; // (原为3)
    if (op == '+') return 7; // 闭包优先级最高
    
    auto it = isp.find(op);
    if (it == isp.end()) throw RegexSyntaxError("Unknown operator in ISP table: " + std::string(1, op));
    return it->second;
}

int InfixToPostfix::getICP(char op) {
    // 栈外优先级 (In-Coming Priority)
    static const std::map<char, int> icp = {
        {'|', 2},  // 优先级最低 (原为4)
        {'*', 6}, {'?', 6}, 
        {'(', 8}, {')', 1}, {'#', 0}
    };
    
    // 显式连接符优先级需高于 | 但低于 *
    if (op == EXPLICIT_CONCAT_OP) return 4; // (原为2)
    if (op == '+') return 6; 
    
    auto it = icp.find(op);
    if (it == icp.end()) throw RegexSyntaxError("Unknown operator in ICP table: " + std::string(1, op));
    return it->second;
}

void InfixToPostfix::convert() {
    postfix_.clear();
    std::vector<Token> input = infix_;
    input.push_back(Token('#')); 
    std::stack<Token> opStack;
    opStack.push(Token('#'));

    size_t i = 0;
    while (i < input.size()) {
        const Token& token = input[i];
        if (token.isOperand()) {
            postfix_.push_back(token);
            ++i;
        } else {
            char c2 = token.opVal;
            // 安全性检查：栈不应为空
            if (opStack.empty()) {
                throw RegexSyntaxError("Internal Error: Operator stack empty during conversion.");
            }
            char c1 = opStack.top().opVal;

            if (getISP(c1) < getICP(c2)) {
                opStack.push(token);
                ++i;
            } else if (getISP(c1) > getICP(c2)) {
                postfix_.push_back(opStack.top());
                opStack.pop();
            } else {
                if (c1 == '#' && c2 == '#') {
                    // 正常结束，跳出循环
                    break;
                }
                // 匹配括号的情况
                if (c1 == '(' && c2 == ')') {
                    opStack.pop();
                    ++i;
                } else {
                    // 其他相等的优先级通常意味着语法错误（除了#和括号）
                    throw RegexSyntaxError("Mismatched parenthesis or invalid operator sequence.");
                }
            }
        }
    }
    
    // 检查最后栈是否只剩 #
    if (!opStack.empty() && opStack.top().opVal != '#') {
         throw RegexSyntaxError("Unbalanced operators or parentheses in regex.");
    }
}

const std::vector<Token>& InfixToPostfix::getPostfix() const {
    return postfix_;
}