#include "lexer.h"
#include "regex_parser.h"
#include "regex_simplifier.h"
#include <iostream>
#include <queue>
#include <algorithm>
#include <cctype>

void Lexer::addTokenClass(const std::string& name, const std::string& regex) {
    TokenClass tc;
    tc.id = tokenClasses_.size();
    tc.name = name;
    tc.regex = regex;
    tokenClasses_.push_back(tc);
}

/**
 * 初始化预定义的 Token 类型（基于 lang.l）
 */
void Lexer::initializeDefaultTokenClasses() {
    // 注意：顺序决定优先级！关键字必须在标识符之前
    
    // 关键字（具体的字符串优先级最高）
    addTokenClass("TM_VAR", "\"var\"");
    addTokenClass("TM_IF", "\"if\"");
    addTokenClass("TM_THEN", "\"then\"");
    addTokenClass("TM_ELSE", "\"else\"");
    addTokenClass("TM_WHILE", "\"while\"");
    addTokenClass("TM_DO", "\"do\"");
    addTokenClass("TM_FOR", "\"for\"");
    addTokenClass("TM_CONTINUE", "\"continue\"");
    addTokenClass("TM_BREAK", "\"break\"");
    addTokenClass("TM_RETURN", "\"return\"");
    addTokenClass("TM_FUNC", "\"func\"");
    addTokenClass("TM_PROC", "\"proc\"");
    
    // 多字符运算符（长的优先，必须在单字符运算符之前）
    addTokenClass("TM_LE", "\"<=\"");
    addTokenClass("TM_GE", "\">=\"");
    addTokenClass("TM_EQ", "\"==\"");
    addTokenClass("TM_NE", "\"!=\"");
    addTokenClass("TM_AND", "\"&&\"");
    addTokenClass("TM_OR", "\"||\"");
    addTokenClass("TM_PLUSEQ", "\"+=\"");
    addTokenClass("TM_MINUSEQ", "\"-=\"");
    addTokenClass("TM_MULEQ", "\"*=\"");
    addTokenClass("TM_DIVEQ", "\"/=\"");
    
    // 浮点数（必须在整数之前）
    // 修正说明：移除了之前正则表达式中意外引入的空格
    // 格式1: digits.digits[exponent]  -> [0-9]+"."[0-9]*...
    // 格式2: .digits[exponent]        -> "."[0-9]+...
    // 格式3: digits exponent          -> [0-9]+exponent...
    addTokenClass("TM_FLOAT", 
        "(([0-9]+\".\"[0-9]*((\"e\"|\"E\")(\"+\"|\"-\")?[0-9]+)?)|"
        "(\".\"[0-9]+((\"e\"|\"E\")(\"+\"|\"-\")?[0-9]+)?)|"
        "([0-9]+((\"e\"|\"E\")(\"+\"|\"-\")?[0-9]+)))");

    // 整数（使用字符类简化）
    addTokenClass("TM_NAT", "[0-9]+");
    
    // 标识符
    addTokenClass("TM_IDENT", "([_A-Za-z][_A-Za-z0-9]*)");
    
    // 单字符运算符
    addTokenClass("TM_SEMICOL", "\";\"");
    addTokenClass("TM_LEFT_PAREN", "\"(\"");
    addTokenClass("TM_RIGHT_PAREN", "\")\"");
    addTokenClass("TM_LEFT_BRACE", "\"{\"");
    addTokenClass("TM_RIGHT_BRACE", "\"}\"");
    addTokenClass("TM_PLUS", "\"+\"");
    addTokenClass("TM_MINUS", "\"-\"");
    addTokenClass("TM_MUL", "\"*\"");
    addTokenClass("TM_DIV", "\"/\"");
    addTokenClass("TM_MOD", "\"%\"");
    addTokenClass("TM_LT", "\"<\"");
    addTokenClass("TM_GT", "\">\"");
    addTokenClass("TM_ASGNOP", "\"=\"");
    addTokenClass("TM_NOT", "\"!\"");
    addTokenClass("TM_AMPERSAND", "\"&\"");
    addTokenClass("TM_COMMA", "\",\"");
    
    // 空白字符
    addTokenClass("TM_BLANK", "(\" \"|\"\\t\"|\"\\n\"|\"\\r\")");
}

void Lexer::build() {
    if (tokenClasses_.empty()) {
        throw std::runtime_error("No token classes defined");
    }
    
    std::cout << "\n=== Building Lexer ===" << std::endl;
    std::cout << "Token Classes: " << tokenClasses_.size() << std::endl;
    
    // Step 1: 为每个 token class 构建 NFA
    std::vector<NFAUnit> nfas;
    std::vector<int> endNodeIds;
    
    for (const auto& tc : tokenClasses_) {
        std::cout << "  Processing [" << tc.id << "]: " << tc.name;
        if (tc.regex.length() > 50) {
            std::cout << " = " << tc.regex.substr(0, 47) << "..." << std::endl;
        } else {
            std::cout << " = " << tc.regex << std::endl;
        }
        
        try {
            // 预处理正则表达式
            auto tokens = preprocessRegex(tc.regex);
            
            // 简化正则表达式
            auto simplifiedTokens = simplifyRegex(tokens);
            
            // 插入连接符
            auto tokensWithConcat = insertConcatSymbols(simplifiedTokens);
            
            // 转换为后缀表达式
            InfixToPostfix converter(tokensWithConcat);
            converter.convert();
            const auto& postfix = converter.getPostfix();
            
            // 构建 NFA
            NFAUnit nfa = regexToNFA(postfix);
            nfas.push_back(nfa);
            endNodeIds.push_back(nfa.end->id);
            
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to build NFA for '" + tc.name + "': " + e.what());
        }
    }
    
    // Step 2: 合并多个 NFA 为一个 NFA
    auto mergedStart = std::make_shared<NodeImpl>(9999, "merged_start");
    
    NFAUnit mergedNFA;
    mergedNFA.start = mergedStart;
    mergedNFA.end = nullptr;
    mergedNFA.edges = {};
    
    for (size_t i = 0; i < nfas.size(); ++i) {
        CharSet epsilon;
        epsilon.isEpsilon = true;
        mergedNFA.edges.push_back({mergedStart, nfas[i].start, epsilon});
        mergedNFA.edges.insert(mergedNFA.edges.end(), 
                               nfas[i].edges.begin(), 
                               nfas[i].edges.end());
    }
    
    std::cout << "\nMerged NFA: " << mergedNFA.edges.size() << " edges" << std::endl;
    
    // Step 3: NFA 转 DFA
    buildDFAFromNFA(mergedNFA, dfaStates_, dfaTransitions_);
    
    // Step 4: 标记接受状态
    acceptStateToTokenClasses_.clear();
    
    for (const auto& dfaState : dfaStates_) {
        std::vector<int> matchedTokenClasses;
        
        for (size_t i = 0; i < endNodeIds.size(); ++i) {
            if (dfaState.nfaStates.count(endNodeIds[i])) {
                matchedTokenClasses.push_back(i);
            }
        }
        
        if (!matchedTokenClasses.empty()) {
            std::sort(matchedTokenClasses.begin(), matchedTokenClasses.end());
            acceptStateToTokenClasses_[dfaState.id] = matchedTokenClasses;
        }
    }
    
    std::cout << "DFA built: " << dfaStates_.size() << " states, " 
              << dfaTransitions_.size() << " transitions" << std::endl;
    std::cout << "Accept states: " << acceptStateToTokenClasses_.size() << std::endl;
    
    isBuilt_ = true;
}

int Lexer::getTokenClassForState(int stateId) const {
    auto it = acceptStateToTokenClasses_.find(stateId);
    if (it == acceptStateToTokenClasses_.end() || it->second.empty()) {
        return -1;
    }
    return it->second[0];
}

std::vector<LexerToken> Lexer::tokenize(const std::string& input) {
    if (!isBuilt_) {
        throw std::runtime_error("Lexer not built. Call build() first.");
    }
    
    std::vector<LexerToken> tokens;
    size_t pos = 0;
    int line = 1, column = 1;
    
    while (pos < input.length()) {
        int currentState = 0;
        int lastAcceptPos = -1;
        int lastAcceptTokenClass = -1;
        size_t i = pos;
        int tempLine = line, tempColumn = column;
        
        while (i < input.length()) {
            char c = input[i];
            
            int nextState = -1;
            for (const auto& trans : dfaTransitions_) {
                if (trans.fromStateId == currentState && trans.transitionSymbol.match(c)) {
                    nextState = trans.toStateId;
                    break;
                }
            }
            
            if (nextState == -1) {
                break;
            }
            
            currentState = nextState;
            i++;
            
            int tokenClassId = getTokenClassForState(currentState);
            if (tokenClassId >= 0) {
                lastAcceptPos = i;
                lastAcceptTokenClass = tokenClassId;
            }
        }
        
        if (lastAcceptPos > static_cast<int>(pos)) {
            std::string lexeme = input.substr(pos, lastAcceptPos - pos);
            
            if (tokenClasses_[lastAcceptTokenClass].name != "TM_BLANK") {
                LexerToken token;
                token.lexeme = lexeme;
                token.tokenClassId = lastAcceptTokenClass;
                token.tokenClassName = tokenClasses_[lastAcceptTokenClass].name;
                token.line = line;
                token.column = column;
                tokens.push_back(token);
            }
            
            for (size_t j = pos; j < static_cast<size_t>(lastAcceptPos); ++j) {
                if (input[j] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
            }
            
            pos = lastAcceptPos;
        } else {
            std::string errorContext = input.substr(pos, std::min(size_t(20), input.length() - pos));
            throw std::runtime_error(
                "Lexical error at line " + std::to_string(line) + 
                ", column " + std::to_string(column) + 
                ": unexpected character '" + std::string(1, input[pos]) + "'\n" +
                "Context: \"" + errorContext + "\""
            );
        }
    }
    
    return tokens;
}

void Lexer::displayDFA() const {
    std::cout << "\n=== Lexer DFA Info ===" << std::endl;
    std::cout << "Total States: " << dfaStates_.size() << std::endl;
    std::cout << "Total Transitions: " << dfaTransitions_.size() << std::endl;
    
    std::cout << "\nAccept States (showing first 20):" << std::endl;
    int count = 0;
    for (const auto& [stateId, tokenClassIds] : acceptStateToTokenClasses_) {
        if (count++ >= 20) {
            std::cout << "  ... and " << (acceptStateToTokenClasses_.size() - 20) << " more" << std::endl;
            break;
        }
        std::cout << "  State " << stateId << " -> ";
        for (size_t i = 0; i < tokenClassIds.size() && i < 3; ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << tokenClasses_[tokenClassIds[i]].name;
            if (i == 0) std::cout << " (priority)";
        }
        if (tokenClassIds.size() > 3) {
            std::cout << " ... (+" << (tokenClassIds.size() - 3) << " more)";
        }
        std::cout << std::endl;
    }
}

void Lexer::generateDotFile(const std::string& filename) const {
    if (!isBuilt_) return;
    
    std::ofstream file(filename);
    if (!file) return;
    
    file << "digraph LexerDFA {\n";
    file << "  rankdir=LR;\n";
    file << "  node [shape=circle, fontsize=10];\n";
    
    if (!dfaStates_.empty()) {
        file << "  __start [shape=none, label=\"\"];\n";
        file << "  __start -> " << dfaStates_[0].id << ";\n";
    }
    
    for (const auto& [stateId, tokenClassIds] : acceptStateToTokenClasses_) {
        file << "  " << stateId << " [shape=doublecircle, label=\"" << stateId;
        if (!tokenClassIds.empty()) {
            std::string name = tokenClasses_[tokenClassIds[0]].name;
            if (name.length() > 15) {
                name = name.substr(0, 12) + "...";
            }
            file << "\\n" << name;
        }
        file << "\"];\n";
    }
    
    std::map<std::pair<int, int>, std::vector<std::string>> aggregated;
    for (const auto& trans : dfaTransitions_) {
        aggregated[{trans.fromStateId, trans.toStateId}].push_back(trans.transitionSymbol.toString());
    }
    
    for (const auto& [key, labels] : aggregated) {
        std::string label;
        for (size_t i = 0; i < labels.size() && i < 5; ++i) {
            if (i > 0) label += ",";
            label += labels[i];
        }
        if (labels.size() > 5) {
            label += ",... ";
        }
        file << "  " << key.first << " -> " << key.second 
             << " [label=\"" << label << "\", fontsize=8];\n";
    }
    
    file << "}\n";
    file.close();
}