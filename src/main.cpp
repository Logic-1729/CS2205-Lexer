#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string regularExpression;
    std::cout << "请输入正则表达式 (支持字符集如 [abc] 或 [a-z], 操作符: ( ) * |): ";
    if (!(std::cin >> regularExpression)) return 0;

    try {
        // Step 1: 预处理
        std::vector<std::string> tokens = preprocessRegex(regularExpression);

        // Step 2: 插入连接符
        std::vector<std::string> tokensWithConcat = insertConcatSymbols(tokens);

        // Step 3: 中缀转后缀
        InfixToPostfix converter(tokensWithConcat);
        converter.convert();
        const std::vector<std::string>& postfix = converter.getPostfix();

        // Step 4: 后缀转 NFA
        NFAUnit nfa = regexToNFA(postfix);

        // Step 5: 可视化 NFA
        displayNFA(nfa);
        generateDotFile_NFA(nfa, "nfa_graph.dot");

        // Step 6: NFA 转 DFA
        std::vector<DFAState> dfaStates;
        std::vector<DFATransition> dfaTransitions;
        buildDFAFromNFA(nfa, dfaStates, dfaTransitions);

        // Step 7: 标记接受状态 (修改为 Int ID 访问)
        int originalNFAEndId = nfa.end->id;

        // Step 8: 可视化 DFA
        displayDFA(dfaStates, dfaTransitions, originalNFAEndId);
        generateDotFile_DFA(dfaStates, dfaTransitions, originalNFAEndId, "dfa_graph.dot");

        std::cout << "流程完成！\n";
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}