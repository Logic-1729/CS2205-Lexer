#include "nfa.h"
#include "dfa.h"
#include <fstream>
#include <iostream>
#include <string>

std::string getDFAStateName(int id, const std::vector<DFAState>& dfaStates) {
    if (id >= 0 && id < dfaStates.size()) return dfaStates[id].stateName;
    return "?";
}

void displayNFA(const NFAUnit& nfa) {
    std::cout << "NFA States (Start: " << nfa.start->debugName 
              << ", End: " << nfa.end->debugName << ")\nTransitions:\n";
    for (const auto& e : nfa.edges) {
        std::cout << "  " << e.startName->debugName 
                  << " --(" << e.symbol.toString() << ")--> " 
                  << e.endName->debugName << "\n";
    }
    std::cout << "End of NFA\n" << std::endl;
}

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                int originalNFAEndId) {
    std::cout << "DFA States:\n";
    for (const auto& state : dfaStates) {
        std::cout << "State " << state.stateName;
        if (state.nfaStates.count(originalNFAEndId)) std::cout << " [Accepting]";
        std::cout << "\n";
    }
    std::cout << "DFA Transitions:\n";
    for (const auto& t : dfaTransitions) {
        std::cout << "  " << getDFAStateName(t.fromStateId, dfaStates) 
                  << " --(" << t.transitionSymbol.toString() << ")--> " 
                  << getDFAStateName(t.toStateId, dfaStates) << "\n";
    }
}

void generateDotFile_NFA(const NFAUnit& nfa, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) return;
    file << "digraph NFA { rankdir=LR; node [shape=circle];\n";
    file << "  " << nfa.end->debugName << " [shape=doublecircle];\n";
    file << "  __start0 [shape=none, label=\"\"]; __start0 -> " << nfa.start->debugName << ";\n";
    for (const auto& e : nfa.edges) {
        file << "  " << e.startName->debugName << " -> " << e.endName->debugName 
             << " [label=\"" << e.symbol.toString() << "\"];\n";
    }
    file << "}\n";
    file.close();
}

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         int originalNFAEndId,
                         const std::string& filename) {
    std::ofstream file(filename);
    if (!file) return;
    file << "digraph DFA { rankdir=LR; node [shape=circle];\n";
    for (const auto& state : dfaStates) {
        if (state.nfaStates.count(originalNFAEndId))
            file << "  " << state.stateName << " [shape=doublecircle];\n";
    }
    file << "  __start0 [shape=none, label=\"\"];\n";
    if (!dfaStates.empty()) file << "  __start0 -> " << dfaStates[0].stateName << ";\n";
    for (const auto& t : dfaTransitions) {
        file << "  " << getDFAStateName(t.fromStateId, dfaStates) << " -> " 
             << getDFAStateName(t.toStateId, dfaStates) 
             << " [label=\"" << t.transitionSymbol.toString() << "\"];\n";
    }
    file << "}\n";
    file.close();
}