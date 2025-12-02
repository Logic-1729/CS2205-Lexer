#include "regex_parser.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// 定义全局常量
const char EXPLICIT_CONCAT_OP = '&';

// 解析 [... ] 内容为 CharSet
CharSet parseCharSet(const std::string& content) {
    CharSet cs;
    cs.isEpsilon = false;
    for (size_t k = 0; k < content. length(); ++k) {
        if (k + 2 < content.length() && content[k+1] == '-') {
            char start = content[k];
            char end = content[k+2];
            if (start > end) {
                throw RegexSyntaxError("Invalid range in character class: " + std::string(1, start) + "-" + std::string(1, end));
            }
            cs.addRange(start, end);
            k += 2;
        } else {
            cs. addRange(content[k], content[k]);
        }
    }
    return cs;
}

// Helper function for handling escape characters
char getEscapedChar(char c) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '0': return '\0';
        case '\\': return '\\';
        case '"': return '"';
        case '\'': return '\'';
        default: return c;
    }
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
                try {
                    tokens.push_back(Token(parseCharSet(content)));
                } catch (const RegexSyntaxError& e) {
                    throw RegexSyntaxError(std::string(e.what()) + " at index " + std::to_string(i));
                }
                i = j; 
            } else {
                throw RegexSyntaxError("Unmatched '[' at index " + std::to_string(i));
            }
        } 
        // Handle String Literals e.g.  "abc"
        else if (c == '"') {
            int j = i + 1;
            while (j < n && re[j] != '"') {
                char current = re[j];
                if (current == '\\') {
                    // Escape sequence found
                    j++;
                    if (j >= n) throw RegexSyntaxError("Unterminated escape sequence at end of string");
                    
                    char escaped = getEscapedChar(re[j]);
                    tokens.push_back(Token(CharSet(escaped)));
                } else {
                    tokens.push_back(Token(CharSet(current)));
                }
                j++;
            }
            if (j >= n) throw RegexSyntaxError("Unterminated string literal starting at index " + std::to_string(i));
            i = j; // Move i to the closing quote
        }
        else if (c == '(' || c == ')' || c == '*' || c == '|' || c == '?' || c == '+') {
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

        // 前一个 token 的类型判断
        bool prevIsUnarySuffix = (prev.isOperator() && (prev.opVal == '*' || prev.opVal == '?' || prev.opVal == '+'));
        bool prevIsCloseParen = (prev.isOperator() && prev.opVal == ')');
        bool prevIsOperand = prev. isOperand();
        
        // 当前 token 的类型判断
        bool currIsOperand = curr.isOperand();
        bool currIsOpenParen = (curr.isOperator() && curr.opVal == '(');

        // 需要插入连接符的情况
        if ((prevIsOperand || prevIsUnarySuffix || prevIsCloseParen) && 
            (currIsOperand || currIsOpenParen)) {
            needConcat = true;
        }

        if (needConcat) {
            result.push_back(Token(EXPLICIT_CONCAT_OP)); 
        }
        result.push_back(curr);
    }
    return result;
}