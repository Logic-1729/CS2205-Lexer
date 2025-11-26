#include "regex_parser.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <algorithm>

// 定义全局常量
const char EXPLICIT_CONCAT_OP = '&';

// 解析 [...] 内容为 CharSet
CharSet parseCharSet(const std::string& content) {
    CharSet cs;
    cs.isEpsilon = false;
    for (size_t k = 0; k < content.length(); ++k) {
        if (k + 2 < content.length() && content[k+1] == '-') {
            char start = content[k];
            char end = content[k+2];
            if (start > end) {
                throw RegexSyntaxError("Invalid range in character class: " + std::string(1, start) + "-" + std::string(1, end));
            }
            cs.addRange(start, end);
            k += 2;
        } else {
            cs.addRange(content[k], content[k]);
        }
    }
    return cs;
}

// Parse escape sequence and return the corresponding character.
// The idx parameter should point to the character immediately after the backslash.
// On return, idx will be advanced past the escape sequence character.
// Throws RegexSyntaxError if the escape sequence is incomplete or unknown.
char parseEscapeSequence(const std::string& str, int& idx) {
    if (idx >= static_cast<int>(str.size())) {
        throw RegexSyntaxError("Incomplete escape sequence at end of string");
    }
    char escaped = str[idx];
    idx++;
    switch (escaped) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\\': return '\\';
        case '"': return '"';
        default:
            throw RegexSyntaxError("Unknown escape sequence: \\" + std::string(1, escaped));
    }
}

std::vector<Token> preprocessRegex(const std::string& re) {
    std::vector<Token> tokens;
    int n = re.size();
    for (int i = 0; i < n; ++i) {
        char c = re[i];
        
        if (c == '"') {
            // String literal: tokenize each character individually
            int j = i + 1;
            while (j < n && re[j] != '"') {
                if (re[j] == '\\') {
                    // Handle escape sequence
                    j++; // Move past backslash
                    char escapedChar = parseEscapeSequence(re, j);
                    tokens.push_back(Token(CharSet(escapedChar)));
                } else {
                    tokens.push_back(Token(CharSet(re[j])));
                    j++;
                }
            }
            if (j >= n) {
                throw RegexSyntaxError("Unmatched '\"' at index " + std::to_string(i));
            }
            i = j; // Move past closing quote
        } else if (c == '[') {
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
                    // 捕获并重新抛出，增加位置信息
                    throw RegexSyntaxError(std::string(e.what()) + " at index " + std::to_string(i));
                }
                i = j; 
            } else {
                throw RegexSyntaxError("Unmatched '[' at index " + std::to_string(i));
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
            result.push_back(Token(EXPLICIT_CONCAT_OP)); 
        }
        result.push_back(curr);
    }
    return result;
}