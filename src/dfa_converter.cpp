/*
 * dfa_converter.cpp - implements the subset construction algorithm to convert an NFA into a DFA.
 * Key features include:
 * - Epsilon-closure computation with per-node caching to avoid redundant DFS traversals.
 * - A 'move' function that computes reachable NFA states from a DFA state on a single input charact
 * - Automatic alphabet extraction from NFA transitions, limited to printable ASCII characters
 * (0-127) to keep the DFA manageable.
 * - BFS-driven DFA state exploration, where each DFA state corresponds to a unique set of NFA state.
 * - Transition deduplication to avoid redundant edges between the same state pair on the same symbol.
 * - Support for 'CharSet'-based symbols (from nfa.h) in transitions, though the current implementation
 * processes transitions character-by-character during construction and creates one 'CharSet' per char.
 */

#include "dfa.h"
#include <queue>
#include <algorithm>
#include <map>
#include <vector>
#include <set>
#include <iostream>

// 全局缓存：NFA节点ID -> Epsilon闭包集合
static std::map<int, std::set<int>> closureCache;

// 计算单个状态的 Epsilon Closure (带缓存的 DFS)
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

// Move操作：给定DFA状态和输入字符，返回目标NFA状态集
DFAState move(const DFAState& state, char inputChar, const NFAUnit& nfa) {
    std::set<int> targetStates;

    for (int nfaStateId : state.nfaStates) {
        for (const Edge& e : nfa.edges) {
            // 检查：非epsilon边 && 起点匹配 && 字符匹配
            if (! e.symbol.isEpsilon &&
                e.startName->id == nfaStateId &&
                e.symbol.match(inputChar)) {
                targetStates.insert(e.endName->id);
            }
        }
    }

    DFAState nextState;
    nextState.nfaStates = targetStates;
    return nextState;
}

// 收集NFA中所有可能的输入字符
std::set<char> collectAlphabet(const NFAUnit& nfa) {
    std::set<char> alphabet;

    for (const Edge& e : nfa.edges) {
        if (!e.symbol. isEpsilon) {
            for (const auto& range : e. symbol.ranges) {
                // 只收集ASCII可打印字符，避免字母表过大
                for (int c = (int)range.start; c <= (int)range. end && c <= 127; c++) {
                    alphabet.insert((char)c);
                }
            }
        }
    }

    return alphabet;
}

// 检查转移是否已存在
bool transitionExists(int fromId, int toId, char c,
                      const std::vector<DFATransition>& transitions) {
    for (const auto& t : transitions) {
        if (t. fromStateId == fromId &&
            t.toStateId == toId &&
            t. transitionSymbol. match(c)) {
            return true;
        }
    }
    return false;
}

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions) {
    closureCache.clear();
    dfaStates.clear();
    dfaTransitions.clear();

    std::map<std::set<int>, int> existingStates;
    int dfaCounter = 0;

    // 创建初始状态
    std::set<int> initSet = {nfa.start->id};
    DFAState initState = epsilonClosure(initSet, nfa);
    initState.id = dfaCounter++;
    initState.stateName = std::to_string(initState.id);

    dfaStates.push_back(initState);
    existingStates[initState.nfaStates] = initState.id;

    // 收集字母表
    std::set<char> alphabet = collectAlphabet(nfa);

    std::cout << "Alphabet size: " << alphabet.size() << " characters" << std::endl;

    // BFS构建DFA
    std::queue<int> workQueue;
    workQueue.push(0);
    std::set<int> processed;

    while (!workQueue.empty()) {
        int currentIdx = workQueue.front();
        workQueue.pop();

        if (processed.count(currentIdx)) continue;
        processed.insert(currentIdx);

        if (currentIdx >= (int)dfaStates.size()) continue;

        DFAState& current = dfaStates[currentIdx];

        // 对每个字符尝试转移
        for (char c : alphabet) {
            DFAState moved = move(current, c, nfa);

            if (! moved.nfaStates. empty()) {
                DFAState closure = epsilonClosure(moved.nfaStates, nfa);

                int targetId;
                auto it = existingStates.find(closure. nfaStates);

                if (it == existingStates.end()) {
                    // 新状态
                    closure.id = dfaCounter++;
                    closure.stateName = std::to_string(closure.id);
                    targetId = closure.id;

                    dfaStates.push_back(closure);
                    existingStates[closure.nfaStates] = targetId;
                    workQueue.push(dfaStates.size() - 1);
                } else {
                    targetId = it->second;
                }

                // 添加转移（避免重复）
                if (! transitionExists(current.id, targetId, c, dfaTransitions)) {
                    CharSet transSymbol(c);
                    dfaTransitions.push_back({current. id, targetId, transSymbol});
                }
            }
        }
    }

    std::cout << "DFA construction complete: " << dfaStates.size()
              << " states, " << dfaTransitions.size() << " transitions" << std::endl;
}
