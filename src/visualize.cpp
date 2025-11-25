#include "nfa.h"
#include "dfa.h"
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <string>

void displayNFA(const NFAUnit& nfa) {
    std::cout << "NFA States:\n";
    std::cout << "Start: " << nfa.start.nodeName << "\n";
    std::cout << "End: " << nfa.end.nodeName << "\n\n";

    std::cout << "Transitions:\n";
    for (size_t i = 0; i < nfa.edges.size(); ++i) {
        const Edge& e = nfa.edges[i];
        std::string label = e.tranSymbol.empty() ? "ε" : e.tranSymbol;
        std::cout << "  " << e.startName.nodeName << " --(" << label << ")--> " << e.endName.nodeName << "\n";
    }
    std::cout << "End of NFA\n" << std::endl;
}

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                const std::string& originalNFAEnd) {
    std::cout << "DFA States:\n";
    for (size_t i = 0; i < dfaStates.size(); ++i) {
        const auto& state = dfaStates[i];
        std::cout << "State " << state.stateName << " (NFA: ";
        for (const auto& s : state.nfaStates) {
            std::cout << s << " ";
        }
        std::cout << ")";
        if (i == 0) {
            std::cout << " [Initial]";
        }
        if (state.nfaStates.find(originalNFAEnd) != state.nfaStates.end()) {
            std::cout << " [Accepting]";
        }
        std::cout << "\n";
    }

    std::cout << "\nDFA Transitions:\n";
    for (const auto& t : dfaTransitions) {
        std::cout << "  " << t.fromState.stateName << " --(" << t.transitionSymbol << ")--> " << t.toState.stateName << "\n";
    }
}

void generateDotFile_NFA(const NFAUnit& nfa, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    file << "digraph NFA {\n";
    file << "  rankdir=LR;\n";
    file << "  node [shape=circle];\n";
    file << "  " << nfa.end.nodeName << " [shape=doublecircle];\n";
    file << "  __start0 [shape=none, label=\"\"];\n";
    file << "  __start0 -> " << nfa.start.nodeName << ";\n\n";

    for (const Edge& e : nfa.edges) {
        std::string label = e.tranSymbol.empty() ? "ε" : e.tranSymbol;
        file << "  " << e.startName.nodeName << " -> " << e.endName.nodeName << " [label=\"" << label << "\"];\n";
    }
    file << "}\n";
    file.close();
    std::cout << "NFA DOT file generated: " << filename << std::endl;
}

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         const std::string& originalNFAEnd,
                         const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    file << "digraph DFA {\n";
    file << "  rankdir=LR;\n";
    file << "  node [shape=circle];\n";

    // 标记所有接受状态：包含 originalNFAEnd 的 DFA 状态
    for (const auto& state : dfaStates) {
        if (state.nfaStates.find(originalNFAEnd) != state.nfaStates.end()) {
            file << "  " << state.stateName << " [shape=doublecircle];\n";
        }
    }

    file << "  __start0 [shape=none, label=\"\"];\n";
    if (!dfaStates.empty()) {
        file << "  __start0 -> " << dfaStates.front().stateName << ";\n";
    }
    file << "\n";

    for (const auto& t : dfaTransitions) {
        file << "  " << t.fromState.stateName << " -> " << t.toState.stateName
             << " [label=\"" << t.transitionSymbol << "\"];\n";
    }
    file << "}\n";
    file.close();
    std::cout << "DFA DOT file generated: " << filename << std::endl;
}