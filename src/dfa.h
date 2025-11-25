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
    int id;                     // DFA 状态的唯一 ID
    std::set<int> nfaStates;    // 对应的 NFA 状态 ID 集合
    std::string stateName;      // 用于可视化，例如 "0" 或 "{q1,q2}"

    // 比较操作符重载，用于 Map/Set 的 key
    bool operator==(const DFAState& other) const {
        return nfaStates == other.nfaStates;
    }

    bool operator<(const DFAState& other) const {
        return nfaStates < other.nfaStates;
    }
};

// DFA 转移
struct DFATransition {
    int fromStateId;    // 使用 ID 记录转移
    int toStateId;      // 使用 ID 记录转移
    std::string transitionSymbol;
};

// ==============================
// NFA → DFA 转换
// ==============================

DFAState epsilonClosure(const std::set<int>& states, const NFAUnit& nfa);
DFAState move(const DFAState& state, const std::string& symbol, const NFAUnit& nfa);

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions);

// ==============================
// DFA 输出与可视化
// ==============================

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                int originalNFAEndId);

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         int originalNFAEndId,
                         const std::string& filename = "dfa.dot");

#endif // DFA_H