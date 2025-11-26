#include "dfa.h"
#include <queue>
#include <algorithm>
#include <map>

// 全局缓存：NFA节点ID -> Epsilon闭包集合
// 注意：在处理新的NFA前应该清空，但本程序一次运行只处理一个正则，故暂不需复杂管理
static std::map<int, std::set<int>> closureCache;

// Helper: 计算单个节点的 epsilon closure（仅自身及直接通过epsilon到达的）
// 实际上，我们可以在需要时计算整个集合的闭包，利用已缓存的单个节点闭包
// 但为了保持现有逻辑结构微调，我们修改 getEpsilonClosure 利用缓存

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

        // 查找从 u 出发的 epsilon 边
        // 优化：如果 nfa 数据结构支持邻接表会更快，目前是遍历所有边 O(E)
        // 建议未来优化 NFAUnit 结构为邻接表
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
    // 对于集合中的每个状态，合并它们的单节点闭包
    for (int id : states) {
        // 如果缓存中没有，先计算（虽然 buildDFA 流程中通常是从已知的闭包扩展，但为了安全）
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

// Compute canonical (disjoint) input intervals from all NFA edge symbols
// This ensures that overlapping ranges are split into non-overlapping intervals
std::vector<CharSet> computeCanonicalIntervals(const NFAUnit& nfa) {
    // Collect all boundary points from all character ranges
    // Use int to avoid overflow when r.end is 255 (max unsigned char)
    std::set<int> boundaries;
    for (const Edge& e : nfa.edges) {
        if (!e.symbol.isEpsilon) {
            for (const auto& r : e.symbol.ranges) {
                boundaries.insert(static_cast<int>(static_cast<unsigned char>(r.start)));
                // Use int arithmetic to avoid overflow when r.end is 255
                boundaries.insert(static_cast<int>(static_cast<unsigned char>(r.end)) + 1);
            }
        }
    }
    
    // Convert to sorted vector
    std::vector<int> sortedBoundaries(boundaries.begin(), boundaries.end());
    
    // Create disjoint intervals between consecutive boundary points
    std::vector<CharSet> intervals;
    for (size_t i = 0; i + 1 < sortedBoundaries.size(); ++i) {
        int start = sortedBoundaries[i];
        int end = sortedBoundaries[i + 1] - 1;
        // Valid character range is [0, 255]
        if (start <= end && start >= 0 && start <= 255 && end <= 255) {
            CharSet cs;
            cs.isEpsilon = false;
            cs.addRange(static_cast<char>(start), static_cast<char>(end));
            intervals.push_back(cs);
        }
    }
    
    return intervals;
}

// Modified move function that checks if edge symbol matches the input interval
// An edge matches if any character in the input interval is accepted by the edge's CharSet
// Precondition: inputInterval must have at least one range (non-empty ranges set)
DFAState moveWithInterval(const DFAState& state, const CharSet& inputInterval, const NFAUnit& nfa) {
    std::set<int> targetStates;
    
    // Safety check: ensure inputInterval has at least one range
    if (inputInterval.ranges.empty()) {
        DFAState emptyState;
        return emptyState;
    }
    
    // Get a representative character from the input interval
    // Since intervals are canonical (disjoint), we just need to pick any character
    char representativeChar = inputInterval.ranges.begin()->start;
    
    for (int nfaStateId : state.nfaStates) {
        for (const Edge& e : nfa.edges) {
            if (e.startName->id == nfaStateId && !e.symbol.isEpsilon) {
                // Check if the edge's CharSet matches the representative character
                if (e.symbol.match(representativeChar)) {
                    targetStates.insert(e.endName->id);
                }
            }
        }
    }
    DFAState nextState;
    nextState.nfaStates = targetStates;
    return nextState;
}

DFAState move(const DFAState& state, const CharSet& symbol, const NFAUnit& nfa) {
    std::set<int> targetStates;
    for (int nfaStateId : state.nfaStates) {
        for (const Edge& e : nfa.edges) {
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
    // 清空缓存，确保多次调用安全
    closureCache.clear(); 
    
    std::map<std::set<int>, int> existingStates;
    int dfaCounter = 0;

    std::set<int> initSet = {nfa.start->id};
    DFAState initState = epsilonClosure(initSet, nfa);
    initState.id = dfaCounter++;
    initState.stateName = std::to_string(initState.id);
    
    dfaStates.push_back(initState);
    existingStates[initState.nfaStates] = initState.id;

    // Compute canonical (disjoint) input intervals from all NFA edges
    std::vector<CharSet> inputs = computeCanonicalIntervals(nfa);

    for (size_t i = 0; i < dfaStates.size(); ++i) {
        DFAState current = dfaStates[i]; 

        for (const auto& symbol : inputs) {
            // Use the new moveWithInterval function that handles overlapping ranges
            DFAState moved = moveWithInterval(current, symbol, nfa);
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