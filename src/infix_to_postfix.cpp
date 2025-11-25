#include "regex_parser.h"
#include <map>
#include <stack>
#include <stdexcept>

InfixToPostfix::InfixToPostfix(const std::vector<std::string>& infix)
    : infix_(infix) {}

bool InfixToPostfix::isLetter(const std::string& token) {
    return ::isLetter(token); // 调用全局 isLetter
}

int InfixToPostfix::getISP(char op) {
    static const std::map<char, int> isp = {
        {'+', 3}, {'|', 5}, {'*', 7}, {'(', 1}, {')', 8}, {'#', 0}
    };
    auto it = isp.find(op);
    if (it == isp.end()) {
        throw std::runtime_error("Unknown operator in ISP: " + std::string(1, op));
    }
    return it->second;
}

int InfixToPostfix::getICP(char op) {
    static const std::map<char, int> icp = {
        {'+', 2}, {'|', 4}, {'*', 6}, {'(', 8}, {')', 1}, {'#', 0}
    };
    auto it = icp.find(op);
    if (it == icp.end()) {
        throw std::runtime_error("Unknown operator in ICP: " + std::string(1, op));
    }
    return it->second;
}

void InfixToPostfix::convert() {
    postfix_.clear();
    std::vector<std::string> input = infix_;
    input.push_back("#");
    std::stack<std::string> opStack;
    opStack.push("#");

    size_t i = 0;
    while (i < input.size()) {
        const std::string& token = input[i];
        if (isLetter(token)) {
            postfix_.push_back(token);
            ++i;
        } else {
            char c2 = token[0];
            char c1 = opStack.top()[0];

            if (getISP(c1) < getICP(c2)) {
                opStack.push(token);
                ++i;
            } else if (getISP(c1) > getICP(c2)) {
                postfix_.push_back(opStack.top());
                opStack.pop();
            } else {
                if (c1 == '#' && c2 == '#') {
                    break;
                }
                opStack.pop();
                ++i;
            }
        }
    }
}

const std::vector<std::string>& InfixToPostfix::getPostfix() const {
    return postfix_;
}