#pragma once

#include <string>
#include <vector>
#include <set>
#include "nfa.h"

struct DFAState {
    int id;
    std::set<int> nfaStates;
    std::string stateName;

    bool operator==(const DFAState& other) const {
        return nfaStates == other.nfaStates;
    }
    bool operator<(const DFAState& other) const {
        return nfaStates < other.nfaStates;
    }
};

struct DFATransition {
    int fromStateId;
    int toStateId;
    CharSet transitionSymbol; // Change string to CharSet
};

DFAState epsilonClosure(const std::set<int>& states, const NFAUnit& nfa);
// move 逻辑现在变得复杂，因为输入不再是单一字符，而需要处理字符集重叠
// 为了简化演示，我们假设 DFA 构建时，遍历所有可能出现的“原子字符集”。
// 或者，我们使用经典的输入字符表驱动。
// 本项目为了简化，我们在 buildDFAFromNFA 中收集所有出现的 CharSet。
// 注意：严格来说，如果 NFA 有 [0-9] 和 [0-5] 两条边，DFA 需要分裂为 [0-5] 和 [6-9]。
// 这里我们采用简化逻辑：只对 NFA 中显式出现的 CharSet 进行转换尝试。
// 这对于不重叠的字符集是正确的。对于重叠字符集，这是一个更复杂的话题（Partition refinement）。
// 建议：在此实现中，我们仅支持 NFA 中出现的原始 CharSet 进行状态转移探测。

DFAState move(const DFAState& state, const CharSet& symbol, const NFAUnit& nfa);

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions);

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                int originalNFAEndId);

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         int originalNFAEndId,
                         const std::string& filename = "dfa.dot");