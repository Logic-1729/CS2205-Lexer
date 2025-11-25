#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::string outputDir = ".";
    if (argc > 1) {
        outputDir = argv[1];
    }

    std::string mkdirCmd;
    if (outputDir[0] == '-') {
        mkdirCmd = "mkdir -p \"./" + outputDir + "\"";
    } else {
        mkdirCmd = "mkdir -p \"" + outputDir + "\"";
    }
    
    if (system(mkdirCmd.c_str()) != 0) {
        std::cerr << "Warning: Failed to create directory '" << outputDir << "'" << std::endl;
    }

    std::string regularExpression;
    if (!(std::cin >> regularExpression)) return 0;

    try {
        // Step 1: 预处理
        std::vector<Token> tokens = preprocessRegex(regularExpression);

        // Step 2: 插入连接符
        std::vector<Token> tokensWithConcat = insertConcatSymbols(tokens);

        // Step 3: 中缀转后缀
        InfixToPostfix converter(tokensWithConcat);
        converter.convert();
        const std::vector<Token>& postfix = converter.getPostfix();

        // Step 4: 后缀转 NFA
        NFAUnit nfa = regexToNFA(postfix);

        // Step 5: 可视化 NFA
        displayNFA(nfa);
        std::string nfaPath = outputDir + "/nfa_graph.dot";
        generateDotFile_NFA(nfa, nfaPath);
        std::cout << "Generated: " << nfaPath << std::endl;

        // Step 6: NFA 转 DFA
        std::vector<DFAState> dfaStates;
        std::vector<DFATransition> dfaTransitions;
        buildDFAFromNFA(nfa, dfaStates, dfaTransitions);

        // Step 7: 标记接受状态
        int originalNFAEndId = nfa.end->id;

        // Step 8: 可视化 DFA
        displayDFA(dfaStates, dfaTransitions, originalNFAEndId);
        std::string dfaPath = outputDir + "/dfa_graph.dot";
        generateDotFile_DFA(dfaStates, dfaTransitions, originalNFAEndId, dfaPath);
        std::cout << "Generated: " << dfaPath << std::endl;

        std::cout << "流程完成！\n";
    
    } catch (const RegexSyntaxError& e) {
        std::cerr << "\n[Syntax Error]: Failed to parse regex.\n";
        std::cerr << "  Reason: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n[System Error]: An unexpected error occurred.\n";
        std::cerr << "  Details: " << e.what() << "\n";
        return 1;
    }

    return 0;
}