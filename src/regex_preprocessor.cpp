#include "regex_parser.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// ==========================================
// 全局常量定义
// ==========================================
// 使用 '&' 作为内部连接符，避免与 '+' (一次或多次) 冲突
const char EXPLICIT_CONCAT_OP = '&';

// 解析 [...] 内容为 CharSet
CharSet parseCharSet(const std::string& content) {
    CharSet cs;
    cs.isEpsilon = false;
    for (size_t k = 0; k < content.length(); ++k) {
        if (k + 2 < content.length() && content[k+1] == '-') {
            char start = content[k];
            char end = content[k+2];
            cs.addRange(start, end);
            k += 2;
        } else {
            cs.addRange(content[k], content[k]);
        }
    }
    return cs;
}

std::vector<Token> preprocessRegex(const std::string& re) {
    std::vector<Token> tokens;
    int n = re.size();
    for (int i = 0; i < n; ++i) {
        char c = re[i];
        
        if (c == '[') {
            std::string content;
            int j = i + 1;
            while (j < n && re[j] != ']') {
                content += re[j];
                j++;
            }
            if (j < n) {
                tokens.push_back(Token(parseCharSet(content)));
                i = j; 
            } else {
                throw std::runtime_error("Unmatched '[' in regex");
            }
        } else if (c == '(' || c == ')' || c == '*' || c == '|' || c == '?' || c == '+') {
            tokens.push_back(Token(c));
        } else {
            tokens.push_back(Token(CharSet(c)));
        }
    }
    return tokens;
}

std::vector<Token> insertConcatSymbols(const std::vector<Token>& tokens) {
    if (tokens.empty()) return {};

    std::vector<Token> result;
    result.push_back(tokens[0]);

    for (size_t i = 1; i < tokens.size(); ++i) {
        const Token& prev = tokens[i - 1];
        const Token& curr = tokens[i];

        bool needConcat = false;

        // 规则：
        // 1. Operand 后面接 Operand 或 '('
        // 2. ) * ? + 后面接 Operand 或 '('
        bool prevIsUnarySuffix = (prev.isOperator() && (prev.opVal == '*' || prev.opVal == '?' || prev.opVal == '+'));
        bool prevIsCloseParen = (prev.isOperator() && prev.opVal == ')');
        bool prevIsOperand = prev.isOperand();
        
        bool currIsOperand = curr.isOperand();
        bool currIsOpenParen = (curr.isOperator() && curr.opVal == '(');

        if ((prevIsOperand || prevIsUnarySuffix || prevIsCloseParen) && 
            (currIsOperand || currIsOpenParen)) {
            needConcat = true;
        }

        if (needConcat) {
            // 使用全局常量
            result.push_back(Token(EXPLICIT_CONCAT_OP)); 
        }
        result.push_back(curr);
    }
    return result;
}