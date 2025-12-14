/*
* regex_simplifier.cpp - implements a regex simplifier that rewrites syntactic sugar operators
 * ('?' for optional and '+' for one-or-more) into their equivalent forms using only core
 * operators (*, |, parentheses, and explicit concatenation). It features:
 * - '?' is expanded as `(X|ε)`, where ε is represented by an epsilon `CharSet` token.
 * - '+' is expanded as `XX*` (i.e., one occurrence followed by Kleene star).
 * - The input is a token stream (from preprocessing) that may contain '?', '+', etc.
 * - The output is a token stream containing only the primitive operators supported
 * by the NFA builder: '*', '|', '(', ')', and the explicit concatenation operator.
 * - isSimplified: checks whether a token sequence contains only these primitive operators,
 * indicating it is ready for NFA construction.
 * - This simplification step enables the NFA builder to handle only a minimal set of operators
 *  while still supporting common regex extensions.
 */
#include "regex_simplifier.h"
#include <stack>
#include <stdexcept>

std::vector<Token> simplifyRegex(const std::vector<Token>& tokens) {
    std::vector<Token> result;
    
    for (size_t i = 0; i < tokens. size(); ++i) {
        const Token& token = tokens[i];
        
        if (token.isOperator()) {
            char op = token.opVal;
            
            if (op == '? ') {
                // X? => (X|ε)
                if (result.empty()) {
                    throw RegexSyntaxError("?  operator without preceding operand");
                }
                
                Token operand = result.back();
                result.pop_back();
                
                // 构造 (X|ε)
                result. push_back(Token('('));
                result.push_back(operand);
                result.push_back(Token('|'));
                
                // 创建 epsilon token
                CharSet epsilon;
                epsilon.isEpsilon = true;
                result.push_back(Token(epsilon));
                
                result.push_back(Token(')'));
                
            } else if (op == '+') {
                // X+ => XX*
                if (result. empty()) {
                    throw RegexSyntaxError("+ operator without preceding operand");
                }
                
                Token operand = result.back();
                // 不弹出，因为需要 XX*
                
                result.push_back(operand); // 添加第二个 X
                result.push_back(Token('*'));
                
            } else {
                // 其他操作符 (*, |, (, ), &) 直接保留
                result.push_back(token);
            }
        } else {
            // 操作数直接添加
            result.push_back(token);
        }
    }
    
    return result;
}

bool isSimplified(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        if (token.isOperator()) {
            char op = token.opVal;
            // 简化正则只允许 *, |, (, ), & (连接符)
            if (op != '*' && op != '|' && op != '(' && op != ')' && op != EXPLICIT_CONCAT_OP) {
                return false;
            }
        }
    }
    return true;
}