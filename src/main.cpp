#include "lexer.h"
#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

// 函数声明更新，增加输出目录参数
void runLexerMode();
void runSingleRegexMode(const std::string& outputDir);
void runPredefinedLexerMode();

int main(int argc, char* argv[]) {
    // 默认参数
    int choice = 0;
    std::string outputDir = ".";

    // 1. 尝试从命令行参数读取模式 (argv[1])
    if (argc > 1) {
        try {
            choice = std::stoi(argv[1]);
        } catch (...) {
            choice = 0;
        }
    }

    // 2. 尝试从命令行参数读取输出目录 (argv[2])，仅用于模式 3
    if (argc > 2) {
        outputDir = argv[2];
    }

    // 如果没有通过命令行指定模式，则显示交互菜单
    if (choice == 0) {
        std::cout << "===========================================\n";
        std::cout << "  CS2205 Lexical Analyzer\n";
        std::cout << "===========================================\n\n";
        
        std::cout << "Select mode:\n";
        std::cout << "  1. Predefined Lexer (lang.l tokens)\n";
        std::cout << "  2. Custom Lexer (define your own tokens)\n";
        std::cout << "  3. Single Regex (Regex -> NFA -> DFA)\n";
        std::cout << "Enter choice (1-3): ";
        
        if (!(std::cin >> choice)) return 0;
        std::cin.ignore(); // 清除换行符
    }

    try {
        switch (choice) {
            case 1:
                runPredefinedLexerMode();
                break;
            case 2:
                runLexerMode();
                break;
            case 3:
                runSingleRegexMode(outputDir);
                break;
            default:
                std::cout << "Invalid choice.\n";
                return 1;
        }
    } catch (const RegexSyntaxError& e) {
        std::cerr << "\n[Syntax Error]: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n[Error]: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

void runPredefinedLexerMode() {
    std::cout << "\n=== Predefined Lexer Mode (lang.l) ===\n";
    
    Lexer lexer;
    lexer.initializeDefaultTokenClasses();
    
    std::cout << "\nBuilding lexer with " << lexer.getTokenClasses().size() << " token types...\n";
    lexer.build();
    // lexer.displayDFA(); // 可选：减少输出干扰
    
    // 生成可视化
    lexer.generateDotFile("lexer_dfa.dot");
    std::cout << "\nGenerated: lexer_dfa.dot\n";
    
    // 词法分析
    std::cout << "\n=== Tokenization ===\n";
    std::cout << "Enter code to analyze (or 'quit' to exit):\n";
    
    std::string input;
    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, input)) break; // 检测 EOF
        if (input == "quit" || input == "exit") break;
        
        if (input.empty()) continue;
        
        try {
            auto tokens = lexer.tokenize(input);
            
            std::cout << "\nTokens:\n";
            // 简单的表格边框
            std::cout << "┌──────┬────────┬──────────────────┬──────────────────────┐\n";
            std::cout << "│ Line │ Column │ Token Type       │ Lexeme                 │\n";
            std::cout << "├──────┼────────┼──────────────────┼──────────────────────┤\n";
            
            for (const auto& token : tokens) {
                std::cout << "│ " << std::setw(4) << token.line 
                         << " │ " << std::setw(6) << token.column
                         << " │ " << std::setw(16) << std::left << token.tokenClassName
                         << " │ " << std::setw(22) << ("\"" + token.lexeme + "\"")
                         << " │\n";
            }
            
            std::cout << "└──────┴────────┴──────────────────┴──────────────────────┘\n";
            std::cout << "Total: " << tokens.size() << " tokens\n";
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

void runLexerMode() {
    std::cout << "\n=== Custom Lexer Mode ===\n";
    
    Lexer lexer;
    
    // 注意：如果通过脚本管道输入，这些提示用户可能看不到，但这不影响程序运行
    std::cout << "Enter number of token classes: ";
    int n;
    if (!(std::cin >> n)) return;
    std::cin.ignore();
    
    for (int i = 0; i < n; ++i) {
        std::cout << "\nToken Class " << (i + 1) << ":\n";
        std::cout << "  Name: ";
        std::string name;
        std::getline(std::cin, name);
        
        std::cout << "  Regex: ";
        std::string regex;
        std::getline(std::cin, regex);
        
        lexer.addTokenClass(name, regex);
    }
    
    lexer.build();
    // lexer.displayDFA();
    lexer.generateDotFile("custom_lexer_dfa.dot");
    
    std::cout << "\n=== Tokenization ===\n";
    std::cout << "Enter input (or 'quit' to exit):\n";
    
    std::string input;
    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, input)) break; // 检测 EOF
        if (input == "quit") break;
        
        try {
            auto tokens = lexer.tokenize(input);
            for (const auto& token : tokens) {
                std::cout << "[" << token.line << ":" << token.column << "] "
                         << token.tokenClassName << " \"" << token.lexeme << "\"\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

void runSingleRegexMode(const std::string& outputDir) {
    std::cout << "\n=== Single Regex Mode ===\n";
    std::cout << "Output Directory: " << outputDir << "\n";
    std::cout << "Enter regular expression: ";
    
    std::string regularExpression;
    if (!std::getline(std::cin, regularExpression)) return;

    // 简单的路径拼接助手
    auto getPath = [&](const std::string& file) {
        // 移除 outputDir 末尾的斜杠（如果有）
        std::string dir = outputDir;
        if (!dir.empty() && dir.back() == '/') dir.pop_back();
        return dir + "/" + file;
    };

    auto tokens = preprocessRegex(regularExpression);
    auto tokensWithConcat = insertConcatSymbols(tokens);
    
    InfixToPostfix converter(tokensWithConcat);
    converter.convert();
    const auto& postfix = converter.getPostfix();

    NFAUnit nfa = regexToNFA(postfix);
    
    std::cout << "\n=== NFA ===" << std::endl;
    displayNFA(nfa);
    std::string nfaPath = getPath("nfa_graph.dot");
    generateDotFile_NFA(nfa, nfaPath);
    std::cout << "Generated: " << nfaPath << std::endl;

    std::vector<DFAState> dfaStates;
    std::vector<DFATransition> dfaTransitions;
    buildDFAFromNFA(nfa, dfaStates, dfaTransitions);
    int originalNFAEndId = nfa.end->id;

    std::cout << "\n=== DFA ===" << std::endl;
    displayDFA(dfaStates, dfaTransitions, originalNFAEndId);
    std::string dfaPath = getPath("dfa_graph.dot");
    generateDotFile_DFA(dfaStates, dfaTransitions, originalNFAEndId, dfaPath);
    std::cout << "Generated: " << dfaPath << std::endl;

    std::vector<DFAState> minDfaStates;
    std::vector<DFATransition> minDfaTransitions;
    minimizeDFA(dfaStates, dfaTransitions, originalNFAEndId, minDfaStates, minDfaTransitions);

    std::cout << "\n=== Minimized DFA ===" << std::endl;
    displayDFA(minDfaStates, minDfaTransitions, originalNFAEndId);
    std::string minDfaPath = getPath("min_dfa_graph.dot");
    generateDotFile_DFA(minDfaStates, minDfaTransitions, originalNFAEndId, minDfaPath);
    std::cout << "Generated: " << minDfaPath << std::endl;
}