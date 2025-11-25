#include "nfa.h"
#include "dfa.h"
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <algorithm>

// 辅助：通过ID查找DFA状态名
std::string getDFAStateName(int id, const std::vector<DFAState>& dfaStates) {
    if (id >= 0 && id < dfaStates.size()) {
        return dfaStates[id].stateName;
    }
    return "?";
}

void displayNFA(const NFAUnit& nfa) {
    std::cout << "NFA States:\n";
    std::cout << "Start: " << nfa.start->debugName << "\n";
    std::cout << "End: " << nfa.end->debugName << "\n\n";

    std::cout << "Transitions:\n";
    for (size_t i = 0; i < nfa.edges.size(); ++i) {
        const Edge& e = nfa.edges[i];
        std::string label = e.tranSymbol.empty() ? "ε" : e.tranSymbol;
        std::cout << "  " << e.startName->debugName << " --(" << label << ")--> " << e.endName->debugName << "\n";
    }
    std::cout << "End of NFA\n" << std::endl;
}

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                int originalNFAEndId) {
    std::cout << "DFA States:\n";
    for (size_t i = 0; i < dfaStates.size(); ++i) {
        const auto& state = dfaStates[i];
        std::cout << "State " << state.stateName << " (NFA IDs: {";
        for (int id : state.nfaStates) {
            std::cout << id << ",";
        }
        std::cout << "})";
        if (i == 0) {
            std::cout << " [Initial]";
        }
        if (state.nfaStates.find(originalNFAEndId) != state.nfaStates.end()) {
            std::cout << " [Accepting]";
        }
        std::cout << "\n";
    }

    std::cout << "\nDFA Transitions:\n";
    for (const auto& t : dfaTransitions) {
        std::string fromName = getDFAStateName(t.fromStateId, dfaStates);
        std::string toName = getDFAStateName(t.toStateId, dfaStates);
        std::cout << "  " << fromName << " --(" << t.transitionSymbol << ")--> " << toName << "\n";
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
    file << "  " << nfa.end->debugName << " [shape=doublecircle];\n"; 
    file << "  __start0 [shape=none, label=\"\"];\n";
    file << "  __start0 -> " << nfa.start->debugName << ";\n\n"; 

    for (const Edge& e : nfa.edges) {
        std::string label = e.tranSymbol.empty() ? "ε" : e.tranSymbol;
        file << "  " << e.startName->debugName << " -> " << e.endName->debugName << " [label=\"" << label << "\"];\n";
    }
    file << "}\n";
    file.close();
    std::cout << "NFA DOT file generated: " << filename << std::endl;
}

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         int originalNFAEndId,
                         const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    file << "digraph DFA {\n";
    file << "  rankdir=LR;\n";
    file << "  node [shape=circle];\n";

    for (const auto& state : dfaStates) {
        if (state.nfaStates.find(originalNFAEndId) != state.nfaStates.end()) {
            file << "  " << state.stateName << " [shape=doublecircle];\n";
        }
    }

    file << "  __start0 [shape=none, label=\"\"];\n";
    if (!dfaStates.empty()) {
        file << "  __start0 -> " << dfaStates.front().stateName << ";\n";
    }
    file << "\n";

    for (const auto& t : dfaTransitions) {
        std::string fromName = getDFAStateName(t.fromStateId, dfaStates);
        std::string toName = getDFAStateName(t.toStateId, dfaStates);
        file << "  " << fromName << " -> " << toName
             << " [label=\"" << t.transitionSymbol << "\"];\n";
    }
    file << "}\n";
    file.close();
    std::cout << "DFA DOT file generated: " << filename << std::endl;
}