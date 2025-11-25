#include "nfa_dfa_builder.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string regularExpression;
    std::cout << "请输入正则表达式 (支持字符集如 [abc]，操作符: ( ) * |): ";
    if (!(std::cin >> regularExpression)) return 0;

    try {
        // Step 1: 预处理：展开 [abc] 为 (a|b|c)
        std::vector<std::string> tokens = preprocessRegex(regularExpression);

        // Step 2: 插入显式连接符 '+'
        std::vector<std::string> tokensWithConcat = insertConcatSymbols(tokens);

        // Step 3: 中缀 → 后缀
        InfixToPostfix converter(tokensWithConcat);
        converter.convert();
        const std::vector<std::string>& postfix = converter.getPostfix();

        // Step 4: 后缀表达式 → NFA
        NFAUnit nfa = regexToNFA(postfix);

        // Step 5: 显示并生成 NFA 可视化
        displayNFA(nfa);
        generateDotFile_NFA(nfa, "nfa_graph.dot");

        // Step 6: NFA → DFA
        std::vector<DFAState> dfaStates;
        std::vector<DFATransition> dfaTransitions;
        buildDFAFromNFA(nfa, dfaStates, dfaTransitions);

        // Step 7: 标记接受状态所需的原始 NFA 终态名称
        std::string originalNFAEnd = nfa.end.nodeName;

        // Step 8: 显示并生成 DFA 可视化 (修正：传入 originalNFAEnd)
        displayDFA(dfaStates, dfaTransitions, originalNFAEnd);
        generateDotFile_DFA(dfaStates, dfaTransitions, originalNFAEnd, "dfa_graph.dot");

        std::cout << "流程完成！请使用 Graphviz 查看 nfa_graph.dot 和 dfa_graph.dot。\n";
        std::cout << "或者运行根目录下的 build_and_run.sh 脚本直接生成 PNG 图片。\n";
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}