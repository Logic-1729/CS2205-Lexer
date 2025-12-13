/*
 * lexer.h - defines a lexical analyzer (lexer) that supports user-defined token classes
 * specified by regular expressions. It builds a unified DFA from multiple regular expressions
 * (one per token class) to perform efficient lexical analysis. It defines:
 * - TokenClass: represents a named token type with an associated regex pattern.
 * - LexerToken: the output token produced during lexing, containing lexeme, token class info,
 * and position.
 * - Lexer: defines functions of the DFA construction and tokenization logic.
 */

#pragma once

#include "dfa.h"
#include "nfa.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

/**
 * Token 类型定义
 */
struct TokenClass {
    int id;
    std::string name;
    std::string regex;
};

/**
 * 词法分析结果 Token
 */
struct LexerToken {
    std::string lexeme;
    int tokenClassId;
    std::string tokenClassName;
    int line;
    int column;
};

/**
 * 词法分析器类
 */
class Lexer {
public:
    /**
     * 添加自定义 Token 类型
     */
    void addTokenClass(const std::string& name, const std::string& regex);
    
    /**
     * 初始化预定义的 Token 类型（基于 lang.l）
     */
    void initializeDefaultTokenClasses();
    
    /**
     * 构建统一 DFA
     */
    void build();
    
    /**
     * 词法分析
     */
    std::vector<LexerToken> tokenize(const std::string& input);
    
    /**
     * 显示 DFA 信息
     */
    void displayDFA() const;
    
    /**
     * 生成 Graphviz 文件
     */
    void generateDotFile(const std::string& filename) const;
    
    /**
     * 获取 Token 类型列表
     */
    const std::vector<TokenClass>& getTokenClasses() const { return tokenClasses_; }

private:
    std::vector<TokenClass> tokenClasses_;
    std::vector<DFAState> dfaStates_;
    std::vector<DFATransition> dfaTransitions_;
    std::map<int, std::vector<int>> acceptStateToTokenClasses_;
    bool isBuilt_ = false;
    
    int getTokenClassForState(int stateId) const;
};
