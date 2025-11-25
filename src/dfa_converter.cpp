#include "dfa.h"
#include <queue>
#include <algorithm>
#include <map>

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
    // Sort to ensure naming consistency (although set is already ordered)
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

// Helper to check if a transition already exists to avoid duplicates
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
    // Map to quickly lookup if a set of NFA states has already been processed into a DFA state.
    // Key: Set of NFA state names
    // Value: Index in dfaStates vector
    std::map<std::set<std::string>, int> existingStates;

    // Get initial state
    std::set<std::string> initSet = {nfa.start.nodeName};
    DFAState initState = epsilonClosure(initSet, nfa);
    
    dfaStates.push_back(initState);
    existingStates[initState.nfaStates] = 0;

    // Use index-based loop since dfaStates grows during iteration
    for (size_t i = 0; i < dfaStates.size(); ++i) {
        // Use Copy instead of Reference to avoid invalidation when vector reallocates
        DFAState current = dfaStates[i]; 

        // Collect all possible input symbols (non-ε)
        std::set<std::string> symbols;
        for (const Edge& e : nfa.edges) {
            if (!e.tranSymbol.empty()) {
                symbols.insert(e.tranSymbol);
            }
        }

        for (const std::string& symbol : symbols) {
            DFAState moved = move(current, symbol, nfa); // Use the copy 'current'
            
            if (!moved.nfaStates.empty()) {
                DFAState closure = epsilonClosure(moved.nfaStates, nfa);

                // OPTIMIZATION: O(log N) lookup instead of O(N) linear scan
                if (existingStates.find(closure.nfaStates) == existingStates.end()) {
                    int newIndex = static_cast<int>(dfaStates.size());
                    dfaStates.push_back(closure);
                    existingStates[closure.nfaStates] = newIndex;
                }

                // Note: We might be pointing to a 'closure' object that is a copy, 
                // but its content (nfaStates) is what identifies it.
                // Ideally, we could grab the reference from dfaStates using the index if we wanted 
                // exact pointer equality, but our structures use value semantics.

                if (!isTransitionInVector(current, closure, symbol, dfaTransitions)) {
                    dfaTransitions.push_back({current, closure, symbol});
                }
            }
        }
    }
}