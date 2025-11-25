#include "nfa_dfa_builder.h"
#include <cctype>
#include <iostream>

bool isLetter(const std::string& s) {
    if (s.empty()) return false;
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
            int j = i + 1;
            while (j < n && re[j] != ']') ++j;
            if (j < n) {
                tokens.push_back(re.substr(i + 1, j - i - 1));
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

        // 需要插入连接符的情况：
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

    // 调试输出
    std::cout << "After inserting '+' : ";
    for (const auto& t : result) std::cout << t << " ";
    std::cout << std::endl;

    return result;
}