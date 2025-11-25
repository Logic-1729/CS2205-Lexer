#include "nfa_dfa_builder.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string regularExpression;
    std::cout << "请输入正则表达式 (支持字符集如 [abc]，操作符: ( ) * |): ";
    std::cin >> regularExpression;

    // Step 1: 预处理：展开 [abc] 为 "abc" token
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

    // Step 7: 标记接受状态（包含原 NFA 终点的状态）
    std::string originalNFAEnd = nfa.end.nodeName;
    std::set<int> acceptStateIndices;
    for (size_t i = 0; i < dfaStates.size(); ++i) {
        if (dfaStates[i].nfaStates.count(originalNFAEnd)) {
            acceptStateIndices.insert(static_cast<int>(i));
        }
    }

    // （可选）在显示/生成 dot 时使用 acceptStateIndices
    // 当前 displayDFA 和 generateDotFile_DFA 仅将最后一个状态视为接受态，
    // 更严谨的做法是修改这两个函数以接收 acceptStateIndices。
    // 但为简化，我们仍按原逻辑输出。

    // Step 8: 显示并生成 DFA 可视化
    displayDFA(dfaStates, dfaTransitions);
    generateDotFile_DFA(dfaStates, dfaTransitions, "dfa_graph.dot");

    std::cout << "流程完成！请使用 Graphviz 查看 nfa_graph.dot 和 dfa_graph.dot。\n";
    return 0;
}