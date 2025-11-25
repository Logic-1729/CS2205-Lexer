#include "nfa_dfa_builder.h"
#include <queue>

// Helper: get all states reachable via ε from a set
std::set<std::string> getEpsilonClosure(const std::set<std::string>& states, const NFAUnit& nfa) {
    std::set<std::string> closure = states;
    std::queue<std::string> q;
    for (const auto& s : states) q.push(s);

    while (!q.empty()) {
        std::string current = q.front(); q.pop();
        for (const Edge& e : nfa.edges) {
            if (e.startName.nodeName == current && e.tranSymbol.empty()) { // ε-edge
                if (closure.insert(e.endName.nodeName).second) {
                    q.push(e.endName.nodeName);
                }
            }
        }
    }
    return closure;
}

DFAState epsilonClosure(const std::set<std::string>& states, const NFAUnit& nfa) {
    auto closureSet = getEpsilonClosure(states, nfa);
    DFAState state;
    state.nfaStates = closureSet;
    for (const auto& s : closureSet) {
        state.stateName += s;
    }
    return state;
}

DFAState move(const DFAState& state, const std::string& symbol, const NFAUnit& nfa) {
    std::set<std::string> targetStates;
    for (const std::string& nfaState : state.nfaStates) {
        for (const Edge& e : nfa.edges) {
            if (e.startName.nodeName == nfaState && e.tranSymbol == symbol) {
                targetStates.insert(e.endName.nodeName);
            }
        }
    }
    DFAState nextState;
    nextState.nfaStates = targetStates;
    for (const auto& s : targetStates) {
        nextState.stateName += s;
    }
    return nextState;
}

bool isDFAStateInVector(const std::vector<DFAState>& dfaStates, const DFAState& target) {
    for (const auto& state : dfaStates) {
        if (state.nfaStates == target.nfaStates) {
            return true;
        }
    }
    return false;
}

bool isTransitionInVector(const DFAState& from, const DFAState& to,
                          const std::string& symbol,
                          const std::vector<DFATransition>& transitions) {
    for (const auto& t : transitions) {
        if (t.fromState.nfaStates == from.nfaStates &&
            t.toState.nfaStates == to.nfaStates &&
            t.transitionSymbol == symbol) {
            return true;
        }
    }
    return false;
}

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions) {
    // Get initial state
    std::set<std::string> initSet = {nfa.start.nodeName};
    DFAState initState = epsilonClosure(initSet, nfa);
    dfaStates.push_back(initState);

    // Use index-based loop to handle dynamic push_back
    for (size_t i = 0; i < dfaStates.size(); ++i) {
        const DFAState& current = dfaStates[i];

        // Collect all possible input symbols (non-ε)
        std::set<std::string> symbols;
        for (const Edge& e : nfa.edges) {
            if (!e.tranSymbol.empty()) {
                symbols.insert(e.tranSymbol);
            }
        }

        for (const std::string& symbol : symbols) {
            DFAState moved = move(current, symbol, nfa);
            if (!moved.nfaStates.empty()) {
                DFAState closure = epsilonClosure(moved.nfaStates, nfa);

                if (!isDFAStateInVector(dfaStates, closure)) {
                    dfaStates.push_back(closure);
                }

                if (!isTransitionInVector(current, closure, symbol, dfaTransitions)) {
                    dfaTransitions.push_back({current, closure, symbol});
                }
            }
        }
    }
}