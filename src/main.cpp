#include "lexer.h"
#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

void runLexerMode();
void runSingleRegexMode();
void runPredefinedLexerMode();

int main(int argc, char* argv[]) {
    std::cout << "===========================================\n";
    std::cout << "  CS2205 Lexical Analyzer\n";
    std::cout << "===========================================\n\n";
    
    std::cout << "Select mode:\n";
    std::cout << "  1. Predefined Lexer (lang.l tokens)\n";
    std::cout << "  2. Custom Lexer (define your own tokens)\n";
    std::cout << "  3. Single Regex (Regex -> NFA -> DFA)\n";
    std::cout << "Enter choice (1-3): ";
    
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    
    try {
        switch (choice) {
            case 1:
                runPredefinedLexerMode();
                break;
            case 2:
                runLexerMode();
                break;
            case 3:
                runSingleRegexMode();
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
    
    std::cout << "\nBuilding lexer with " << lexer.getTokenClasses().size() << " token types.. .\n";
    lexer.build();
    lexer.displayDFA();
    
    // 生成可视化
    lexer.generateDotFile("lexer_dfa.dot");
    std::cout << "\nGenerated: lexer_dfa.dot\n";
    
    // 词法分析
    std::cout << "\n=== Tokenization ===\n";
    std::cout << "Enter code to analyze (or 'quit' to exit):\n";
    
    std::string input;
    while (true) {
        std::cout << "\n> ";
        if (! std::getline(std::cin, input)) break;
        if (input == "quit" || input == "exit") break;
        
        if (input.empty()) continue;
        
        try {
            auto tokens = lexer.tokenize(input);
            
            std::cout << "\nTokens:\n";
            std::cout << "┌──────┬────────┬──────────────────┬────────────────────────┐\n";
            std::cout << "│ Line │ Column │ Token Type       │ Lexeme                 │\n";
            std::cout << "├──────┼────────┼──────────────────┼────────────────────────┤\n";
            
            for (const auto& token : tokens) {
                std::cout << "│ " << std::setw(4) << token.line 
                         << " │ " << std::setw(6) << token.column
                         << " │ " << std::setw(16) << std::left << token.tokenClassName
                         << " │ " << std::setw(22) << ("\"" + token.lexeme + "\"")
                         << " │\n";
            }
            
            std::cout << "└──────┴────────┴──────────────────┴────────────────────────┘\n";
            std::cout << "Total: " << tokens.size() << " tokens\n";
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

void runLexerMode() {
    std::cout << "\n=== Custom Lexer Mode ===\n";
    
    Lexer lexer;
    
    std::cout << "Enter number of token classes: ";
    int n;
    std::cin >> n;
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
    lexer.displayDFA();
    lexer.generateDotFile("custom_lexer_dfa.dot");
    
    std::cout << "\n=== Tokenization ===\n";
    std::cout << "Enter input (or 'quit' to exit):\n";
    
    std::string input;
    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, input)) break;
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

void runSingleRegexMode() {
    std::cout << "\n=== Single Regex Mode ===\n";
    std::cout << "Enter regular expression: ";
    
    std::string regularExpression;
    if (!std::getline(std::cin, regularExpression)) return;

    auto tokens = preprocessRegex(regularExpression);
    auto tokensWithConcat = insertConcatSymbols(tokens);
    
    InfixToPostfix converter(tokensWithConcat);
    converter.convert();
    const auto& postfix = converter.getPostfix();

    NFAUnit nfa = regexToNFA(postfix);
    
    std::cout << "\n=== NFA ===" << std::endl;
    displayNFA(nfa);
    generateDotFile_NFA(nfa, "nfa_graph.dot");
    std::cout << "Generated: nfa_graph.dot" << std::endl;

    std::vector<DFAState> dfaStates;
    std::vector<DFATransition> dfaTransitions;
    buildDFAFromNFA(nfa, dfaStates, dfaTransitions);
    int originalNFAEndId = nfa.end->id;

    std::cout << "\n=== DFA ===" << std::endl;
    displayDFA(dfaStates, dfaTransitions, originalNFAEndId);
    generateDotFile_DFA(dfaStates, dfaTransitions, originalNFAEndId, "dfa_graph.dot");
    std::cout << "Generated: dfa_graph.dot" << std::endl;

    std::vector<DFAState> minDfaStates;
    std::vector<DFATransition> minDfaTransitions;
    minimizeDFA(dfaStates, dfaTransitions, originalNFAEndId, minDfaStates, minDfaTransitions);

    std::cout << "\n=== Minimized DFA ===" << std::endl;
    displayDFA(minDfaStates, minDfaTransitions, originalNFAEndId);
    generateDotFile_DFA(minDfaStates, minDfaTransitions, originalNFAEndId, "min_dfa_graph.dot");
    std::cout << "Generated: min_dfa_graph. dot" << std::endl;
}