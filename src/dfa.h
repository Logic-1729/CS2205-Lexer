/*
 * dfa.h - declares the data structures and functions for DFA construction
 * from an NFA using subset construction. It defines:
 * - DFAState: a DFA state represented by a unique ID, a set of NFA state IDs it corresponds to,
 * and a human-readable name; it supports comparison via the underlying NFA state set.
 * - DFATransition: a deterministic transition between two DFA states labeled by a 'CharSet'.
 */
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

DFAState move(const DFAState& state, const CharSet& symbol, const NFAUnit& nfa);

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions);

// 新增：DFA 最小化函数
void minimizeDFA(const std::vector<DFAState>& dfaStates,
                 const std::vector<DFATransition>& dfaTransitions,
                 int originalNFAEndId,
                 std::vector<DFAState>& minDfaStates,
                 std::vector<DFATransition>& minDfaTransitions);

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                int originalNFAEndId);

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         int originalNFAEndId,
                         const std::string& filename = "dfa.dot");