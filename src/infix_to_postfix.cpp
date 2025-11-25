#include "regex_parser.h"
#include <map>
#include <stack>
#include <stdexcept>

InfixToPostfix::InfixToPostfix(const std::vector<Token>& infix)
    : infix_(infix) {}

int InfixToPostfix::getISP(char op) {
    static const std::map<char, int> isp = {
        {'|', 5}, 
        {'*', 7}, {'?', 7}, 
        {'(', 1}, {')', 8}, {'#', 0}
    };
    
    if (op == EXPLICIT_CONCAT_OP) return 3; 
    if (op == '+') return 7; 
    
    auto it = isp.find(op);
    if (it == isp.end()) throw RegexSyntaxError("Unknown operator in ISP table: " + std::string(1, op));
    return it->second;
}

int InfixToPostfix::getICP(char op) {
    static const std::map<char, int> icp = {
        {'|', 4}, 
        {'*', 6}, {'?', 6}, 
        {'(', 8}, {')', 1}, {'#', 0}
    };
    
    if (op == EXPLICIT_CONCAT_OP) return 2; 
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