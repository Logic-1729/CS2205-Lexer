#include "regex_parser.h"
#include <map>
#include <stack>
#include <stdexcept>

InfixToPostfix::InfixToPostfix(const std::vector<Token>& infix)
    : infix_(infix) {}

int InfixToPostfix::getISP(char op) {
    // 栈内优先级
    static const std::map<char, int> isp = {
        {'|', 5}, 
        {'*', 7}, {'?', 7}, {'^', 7}, 
        {'(', 1}, {')', 8}, {'#', 0}
    };
    
    // 使用全局常量判断连接符
    if (op == EXPLICIT_CONCAT_OP) return 3; 
    if (op == '+') return 7; // One or more (unary)
    
    auto it = isp.find(op);
    if (it == isp.end()) throw std::runtime_error(std::string("Unknown operator in ISP: ") + op);
    return it->second;
}

int InfixToPostfix::getICP(char op) {
    // 栈外优先级
    static const std::map<char, int> icp = {
        {'|', 4}, 
        {'*', 6}, {'?', 6}, 
        {'(', 8}, {')', 1}, {'#', 0}
    };
    
    // 使用全局常量判断连接符
    if (op == EXPLICIT_CONCAT_OP) return 2; 
    if (op == '+') return 6; // One or more
    
    auto it = icp.find(op);
    if (it == icp.end()) throw std::runtime_error(std::string("Unknown operator in ICP: ") + op);
    return it->second;
}

void InfixToPostfix::convert() {
    postfix_.clear();
    std::vector<Token> input = infix_;
    input.push_back(Token('#')); // End marker
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
            char c1 = opStack.top().opVal;

            if (getISP(c1) < getICP(c2)) {
                opStack.push(token);
                ++i;
            } else if (getISP(c1) > getICP(c2)) {
                postfix_.push_back(opStack.top());
                opStack.pop();
            } else {
                if (c1 == '#' && c2 == '#') break;
                opStack.pop();
                ++i;
            }
        }
    }
}

const std::vector<Token>& InfixToPostfix::getPostfix() const {
    return postfix_;
}