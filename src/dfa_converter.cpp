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

    // 收集所有非 ε 的输入符号 (CharSet)
    std::vector<CharSet> inputs;
    for (const Edge& e : nfa.edges) {
        if (!e.symbol.isEpsilon) {
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