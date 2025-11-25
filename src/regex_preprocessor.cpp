#include "nfa_dfa_builder.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// 判断是否为普通字符（操作数）
bool isLetter(const std::string& s) {
    if (s.empty()) return false;
    
    // 如果是单个字符，只要不是保留的操作符，就是字母
    if (s.length() == 1) {
        char c = s[0];
        return c != '(' && c != ')' && c != '*' && c != '|' && c != '+' && c != '#';
    }
    // 如果长度大于1（例如转义字符或特殊标记），视为字母
    return true; 
}

// 预处理：处理字符集 [a-z] 等
std::vector<std::string> preprocessRegex(const std::string& re) {
    std::vector<std::string> tokens;
    int n = re.size();
    for (int i = 0; i < n; ++i) {
        if (re[i] != '[') {
            tokens.push_back(std::string(1, re[i]));
        } else {
            // 处理 [ ... ]
            std::string charsetContent;
            int j = i + 1;
            while (j < n && re[j] != ']') {
                charsetContent += re[j];
                j++;
            }
            
            if (j < n) {
                if (!charsetContent.empty()) {
                    tokens.push_back("(");
                    
                    // 解析字符集内部，处理 a-z 这种范围
                    std::vector<std::string> expandedChars;
                    for (size_t k = 0; k < charsetContent.length(); ++k) {
                        // 检查是否是范围格式: char - char
                        // 确保 '-' 前后都有字符，且不是第一个或最后一个
                        if (k + 2 < charsetContent.length() && charsetContent[k+1] == '-') {
                            char start = charsetContent[k];
                            char end = charsetContent[k+2];
                            
                            // 简单的 ASCII 范围展开
                            if (start <= end) {
                                for (char c = start; c <= end; ++c) {
                                    expandedChars.push_back(std::string(1, c));
                                }
                            }
                            k += 2; // 跳过 '-' 和 'end'
                        } else {
                            expandedChars.push_back(std::string(1, charsetContent[k]));
                        }
                    }

                    // 将展开后的字符用 '|' 连接
                    for (size_t k = 0; k < expandedChars.size(); ++k) {
                        tokens.push_back(expandedChars[k]);
                        if (k < expandedChars.size() - 1) {
                            tokens.push_back("|");
                        }
                    }
                    
                    tokens.push_back(")");
                }
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