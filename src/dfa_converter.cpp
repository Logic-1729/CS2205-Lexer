#include "dfa.h"
#include <queue>
#include <algorithm>
#include <map>
#include <vector>
#include <set>

// 全局缓存：NFA节点ID -> Epsilon闭包集合
static std::map<int, std::set<int>> closureCache;

// 计算单个状态的 Epsilon Closure (带缓存的 DFS/BFS)
std::set<int> getSingleNodeClosure(int startNodeId, const NFAUnit& nfa) {
    if (closureCache.count(startNodeId)) {
        return closureCache[startNodeId];
    }

    std::set<int> closure;
    std::vector<int> stack;
    
    stack.push_back(startNodeId);
    closure.insert(startNodeId);

    while (!stack.empty()) {
        int u = stack.back();
        stack.pop_back();

        for (const Edge& e : nfa.edges) {
            if (e.startName->id == u && e.symbol.isEpsilon) {
                int v = e.endName->id;
                if (closure.find(v) == closure.end()) {
                    closure.insert(v);
                    stack.push_back(v);
                }
            }
        }
    }

    closureCache[startNodeId] = closure;
    return closure;
}

// 优化后的集合闭包计算
std::set<int> getEpsilonClosure(const std::set<int>& states, const NFAUnit& nfa) {
    std::set<int> result;
    for (int id : states) {
        if (closureCache.find(id) == closureCache.end()) {
            getSingleNodeClosure(id, nfa);
        }
        const auto& singleClosure = closureCache[id];
        result.insert(singleClosure.begin(), singleClosure.end());
    }
    return result;
}

DFAState epsilonClosure(const std::set<int>& states, const NFAUnit& nfa) {
    auto closureSet = getEpsilonClosure(states, nfa);
    DFAState state;
    state.nfaStates = closureSet;
    return state;
}

DFAState move(const DFAState& state, const CharSet& symbol, const NFAUnit& nfa) {
    std::set<int> targetStates;
    // Use a representative character from the disjoint input set to check coverage
    char representative = 0;
    if (!symbol.ranges.empty()) {
        representative = symbol.ranges.begin()->start;
    }

    for (int nfaStateId : state.nfaStates) {
        for (const Edge& e : nfa.edges) {
            // Instead of exact equality (e.symbol == symbol), check if edge covers the input range.
            // Since 'symbol' comes from a disjoint partition of all boundaries, 
            // e.symbol covers 'symbol' iff it matches a representative char.
            if (!e.symbol.isEpsilon && e.startName->id == nfaStateId && e.symbol.match(representative)) {
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

// Helper to generate disjoint canonical inputs from NFA edges
std::vector<CharSet> getCanonicalInputs(const NFAUnit& nfa) {
    std::set<int> points;
    // Collect all interval boundaries
    for (const Edge& e : nfa.edges) {
        if (!e.symbol.isEpsilon) {
            for (const auto& r : e.symbol.ranges) {
                points.insert((int)r.start);
                points.insert((int)r.end + 1);
            }
        }
    }

    std::vector<int> sortedPoints(points.begin(), points.end());
    std::vector<CharSet> inputs;

    for (size_t i = 0; i < sortedPoints.size(); ++i) {
        if (i + 1 < sortedPoints.size()) {
            int start = sortedPoints[i];
            int end = sortedPoints[i+1] - 1;
            if (start <= end) {
                inputs.push_back(CharSet((char)start, (char)end));
            }
        }
    }
    return inputs;
}

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions) {
    closureCache.clear(); 
    
    std::map<std::set<int>, int> existingStates;
    int dfaCounter = 0;

    std::set<int> initSet = {nfa.start->id};
    DFAState initState = epsilonClosure(initSet, nfa);
    initState.id = dfaCounter++;
    initState.stateName = std::to_string(initState.id);
    
    dfaStates.push_back(initState);
    existingStates[initState.nfaStates] = initState.id;

    // Collect disjoint input ranges covering all transitions
    std::vector<CharSet> inputs = getCanonicalInputs(nfa);

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