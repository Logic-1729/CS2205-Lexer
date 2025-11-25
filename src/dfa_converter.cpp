#include "dfa.h"
#include <queue>
#include <algorithm>
#include <map>

std::set<int> getEpsilonClosure(const std::set<int>& states, const NFAUnit& nfa) {
    std::set<int> closure = states;
    std::queue<int> q;
    for (const auto& s : states) q.push(s);

    while (!q.empty()) {
        int currentId = q.front(); q.pop();
        for (const Edge& e : nfa.edges) {
            if (e.startName->id == currentId && e.symbol.isEpsilon) { 
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
    return state;
}

// 修改 move：检查字符集是否有交集。
// 简单实现：如果 edge.symbol == inputSymbol。
// 更高级实现：如果 edge.symbol 与 inputSymbol 有交集。
// 在此我们假设 NFA 构建时，输入符号就是 NFA 边上的那些 CharSet。
DFAState move(const DFAState& state, const CharSet& symbol, const NFAUnit& nfa) {
    std::set<int> targetStates;
    for (int nfaStateId : state.nfaStates) {
        for (const Edge& e : nfa.edges) {
            // 这里比较的是 CharSet 的相等性。
            // 如果要支持重叠区间，需要更复杂的逻辑（区间树或分裂）。
            if (e.startName->id == nfaStateId && e.symbol == symbol) {
                targetStates.insert(e.endName->id);
            }
        }
    }
    DFAState nextState;
    nextState.nfaStates = targetStates;
    return nextState;
}

bool isTransitionInVector(int fromId, int toId,
                          const CharSet& symbol,
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
    std::map<std::set<int>, int> existingStates;
    int dfaCounter = 0;

    std::set<int> initSet = {nfa.start->id};
    DFAState initState = epsilonClosure(initSet, nfa);
    initState.id = dfaCounter++;
    initState.stateName = std::to_string(initState.id);
    
    dfaStates.push_back(initState);
    existingStates[initState.nfaStates] = initState.id;

    // 收集所有非 ε 的输入符号 (CharSet)
    std::vector<CharSet> inputs;
    for (const Edge& e : nfa.edges) {
        if (!e.symbol.isEpsilon) {
            // 简单去重
            bool found = false;
            for(const auto& existing : inputs) if(existing == e.symbol) found = true;
            if(!found) inputs.push_back(e.symbol);
        }
    }

    for (size_t i = 0; i < dfaStates.size(); ++i) {
        DFAState current = dfaStates[i]; 

        for (const auto& symbol : inputs) {
            DFAState moved = move(current, symbol, nfa);
            if (!moved.nfaStates.empty()) {
                DFAState closure = epsilonClosure(moved.nfaStates, nfa);

                auto it = existingStates.find(closure.nfaStates);
                if (it == existingStates.end()) {
                    closure.id = dfaCounter++;
                    closure.stateName = std::to_string(closure.id);
                    dfaStates.push_back(closure);
                    existingStates[closure.nfaStates] = closure.id;
                } else {
                    closure.id = it->second;
                    closure.stateName = dfaStates[it->second].stateName;
                }

                if (!isTransitionInVector(current.id, closure.id, symbol, dfaTransitions)) {
                    dfaTransitions.push_back({current.id, closure.id, symbol});
                }
            }
        }
    }
}