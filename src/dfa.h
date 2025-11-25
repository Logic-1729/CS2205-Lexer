#ifndef DFA_H
#define DFA_H

#include <string>
#include <vector>
#include <set>
#include "nfa.h"

// ==============================
// DFA 结构定义
// ==============================

// DFA 状态
struct DFAState {
    std::set<std::string> nfaStates;
    std::string stateName;

    bool operator==(const DFAState& other) const {
        return nfaStates == other.nfaStates;
    }

    bool operator<(const DFAState& other) const {
        return nfaStates < other.nfaStates;
    }
};

// DFA 转移
struct DFATransition {
    DFAState fromState;
    DFAState toState;
    std::string transitionSymbol;
};

// ==============================
// NFA → DFA 转换
// ==============================

DFAState epsilonClosure(const std::set<std::string>& states, const NFAUnit& nfa);
DFAState move(const DFAState& state, const std::string& symbol, const NFAUnit& nfa);

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions);

// ==============================
// DFA 输出与可视化
// ==============================

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                const std::string& originalNFAEnd);

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         const std::string& originalNFAEnd,
                         const std::string& filename = "dfa.dot");

#endif // DFA_H