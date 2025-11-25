#include "nfa_dfa_builder.h"
#include <cctype>
#include <iostream>
#include <stdexcept>

bool isLetter(const std::string& s) {
    if (s.empty()) return false;
    // 如果是预处理生成的单个字符，通常长度为1
    if (s.length() == 1) {
        char ch = s[0];
        return ( (ch >= 'a' && ch <= 'z') ||
                 (ch >= 'A' && ch <= 'Z') ||
                 (ch >= '0' && ch <= '9') ||
                 ch == '_' || ch == '-' );
    }
    // 兼容旧逻辑，允许多字符作为单一token（如未拆分的字符串）
    for (char ch : s) {
        if (!( (ch >= 'a' && ch <= 'z') ||
               (ch >= 'A' && ch <= 'Z') ||
               (ch >= '0' && ch <= '9') ||
               ch == '_' || ch == '-' )) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> preprocessRegex(const std::string& re) {
    std::vector<std::string> tokens;
    int n = re.size();
    for (int i = 0; i < n; ++i) {
        if (re[i] != '[') {
            tokens.push_back(std::string(1, re[i]));
        } else {
            // 处理 [abc] -> (a|b|c)
            std::string charsetContent;
            int j = i + 1;
            while (j < n && re[j] != ']') {
                charsetContent += re[j];
                j++;
            }
            
            if (j < n) {
                // 找到了闭合的 ]
                if (!charsetContent.empty()) {
                    tokens.push_back("(");
                    for (size_t k = 0; k < charsetContent.length(); ++k) {
                        tokens.push_back(std::string(1, charsetContent[k]));
                        if (k < charsetContent.length() - 1) {
                            tokens.push_back("|");
                        }
                    }
                    tokens.push_back(")");
                }
                i = j; // skip ']'
            } else {
                throw std::runtime_error("Unmatched '[' in regex");
            }
        }
    }
    return tokens;
}

std::vector<std::string> insertConcatSymbols(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return {};

    std::vector<std::string> result;
    result.push_back(tokens[0]);

    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& prev = tokens[i - 1];
        const std::string& curr = tokens[i];

        bool needConcat = false;

        // 1. prev 是字母/字符集 且 curr 是字母/字符集 或 '('
        if (isLetter(prev) && (isLetter(curr) || curr == "(")) {
            needConcat = true;
        }
        // 2. prev 是 ')' 且 curr 是字母/字符集 或 '('
        if (prev == ")" && (isLetter(curr) || curr == "(")) {
            needConcat = true;
        }
        // 3. prev 是 '*' 且 curr 是字母/字符集 或 '('
        if (prev == "*" && (isLetter(curr) || curr == "(")) {
            needConcat = true;
        }

        if (needConcat) {
            result.push_back("+");
        }
        result.push_back(curr);
    }
    return result;
}