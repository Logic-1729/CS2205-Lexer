#include "dfa.h"
#include <queue>
#include <algorithm>
#include <map>

// Helper: get all states reachable via ε from a set of IDs
std::set<int> getEpsilonClosure(const std::set<int>& states, const NFAUnit& nfa) {
    std::set<int> closure = states;
    std::queue<int> q;
    for (const auto& s : states) q.push(s);

    while (!q.empty()) {
        int currentId = q.front(); q.pop();
        for (const Edge& e : nfa.edges) {
            // 使用 ->id 访问
            if (e.startName->id == currentId && e.tranSymbol.empty()) { // ε-edge
                if (closure.insert(e.endName->id).second) {
                    q.push(e.endName->id);
                }
            }
        }
    }
    return closure;
}

DFAState epsilonClosure(const std::set<int>& states, const NFAUnit& nfa) {
    auto closureSet = getEpsilonClosure(states, nfa);
    DFAState state;
    state.nfaStates = closureSet;
    // stateName 不再是必须的核心逻辑，但为了调试可以生成一个简短的名字
    // 这里暂且留空，或者由 buildDFAFromNFA 统一分配
    return state;
}

DFAState move(const DFAState& state, const std::string& symbol, const NFAUnit& nfa) {
    std::set<int> targetStates;
    for (int nfaStateId : state.nfaStates) {
        for (const Edge& e : nfa.edges) {
            if (e.startName->id == nfaStateId && e.tranSymbol == symbol) {
                targetStates.insert(e.endName->id);
            }
        }
    }
    DFAState nextState;
    nextState.nfaStates = targetStates;
    return nextState;
}

bool isTransitionInVector(int fromId, int toId,
                          const std::string& symbol,
                          const std::vector<DFATransition>& transitions) {
    for (const auto& t : transitions) {
        if (t.fromStateId == fromId &&
            t.toStateId == toId &&
            t.transitionSymbol == symbol) {
            return true;
        }
    }
    return false;
}

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions) {
    // Map: NFA状态集合(set<int>) -> DFA状态ID(int)
    std::map<std::set<int>, int> existingStates;
    int dfaCounter = 0;

    // Get initial state
    std::set<int> initSet = {nfa.start->id};
    DFAState initState = epsilonClosure(initSet, nfa);
    initState.id = dfaCounter++;
    initState.stateName = std::to_string(initState.id); // 简单的名字： "0"
    
    dfaStates.push_back(initState);
    existingStates[initState.nfaStates] = initState.id;

    for (size_t i = 0; i < dfaStates.size(); ++i) {
        DFAState current = dfaStates[i]; 

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

                // 检查该状态是否已存在
                auto it = existingStates.find(closure.nfaStates);
                if (it == existingStates.end()) {
                    // 新状态
                    closure.id = dfaCounter++;
                    closure.stateName = std::to_string(closure.id);
                    dfaStates.push_back(closure);
                    existingStates[closure.nfaStates] = closure.id;
                } else {
                    // 已存在，复用 ID
                    closure.id = it->second;
                    // 复用已存在的 stateName
                    closure.stateName = dfaStates[it->second].stateName;
                }

                if (!isTransitionInVector(current.id, closure.id, symbol, dfaTransitions)) {
                    dfaTransitions.push_back({current.id, closure.id, symbol});
                }
            }
        }
    }
}