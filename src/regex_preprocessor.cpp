#include "regex_parser.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// 判断是否为操作数（字母、数字、字符集等）
bool isLetter(const std::string& s) {
    if (s.empty()) return false;
    
    // 如果是单个字符，只要不是保留的操作符，就是操作数
    if (s.length() == 1) {
        char c = s[0];
        return c != '(' && c != ')' && c != '*' && c != '|' && c != '+' && c != '#';
    }
    
    // 如果是多个字符（例如 [a-z] 或转义字符），直接视为操作数
    return true; 
}

// 预处理：将 [a-z] 等视为单个 Token，不展开
std::vector<std::string> preprocessRegex(const std::string& re) {
    std::vector<std::string> tokens;
    int n = re.size();
    for (int i = 0; i < n; ++i) {
        if (re[i] != '[') {
            tokens.push_back(std::string(1, re[i]));
        } else {
            // 处理 [ ... ]，将其整体提取作为一个 Token
            std::string charsetContent = "[";
            int j = i + 1;
            while (j < n && re[j] != ']') {
                charsetContent += re[j];
                j++;
            }
            
            if (j < n) {
                charsetContent += "]";
                tokens.push_back(charsetContent);
                i = j; // 跳过 ']'
            } else {
                throw std::runtime_error("Unmatched '[' in regex");
            }
        }
    }
    return tokens;
}

// 插入隐式连接符 '+'
std::vector<std::string> insertConcatSymbols(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return {};

    std::vector<std::string> result;
    result.push_back(tokens[0]);

    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& prev = tokens[i - 1];
        const std::string& curr = tokens[i];

        bool needConcat = false;

        bool prevIsOperand = isLetter(prev);
        bool currIsOperand = isLetter(curr);

        // 规则：
        // 1. 操作数/右括号/星号 后面接 操作数/左括号 时，需要加连接符
        // (a) (b) -> (a)+(b)
        // a b -> a+b
        // a ( -> a+(
        // ) a -> )+a
        // * a -> *+a
        
        if ((prevIsOperand || prev == ")" || prev == "*") && 
            (currIsOperand || curr == "(")) {
            needConcat = true;
        }

        if (needConcat) {
            result.push_back("+");
        }
        result.push_back(curr);
    }
    return result;
}