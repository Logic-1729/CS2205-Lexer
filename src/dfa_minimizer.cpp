/*
 * dfa_minimizer.cpp - implements a DFA minimization algorithm based on partition refinement.
 * It takes a DFA (represented by `dfaStates` and `dfaTransitions`) and reduces it to
 * an equivalent minimal DFA by merging indistinguishable states. Key steps:
 * - Initial partitioning of states into accepting and non-accepting sets,
 * where acceptance is determined by whether the state's underlying NFA state set
 * contains the original NFA's final state (`originalNFAEndId`).
 * - Iterative refinement of partitions: states in the same partition are split
 * if they exhibit different transition behaviors (i.e., they transition to
 * states in different partitions) under any input symbol from the DFA's alphabet.
 * - Transition signatures are computed per state by recording, for every symbol
 * in the alphabet, the partition index of the target state (or -1 if no transition).
 * - After convergence, a minimized DFA is constructed where each partition becomes
 * a single state, preserving the original start state and acceptance condition.
 * - Transitions in the minimized DFA are derived from a representative state of each
 * partition, with deduplication to avoid duplicate edges.
 * The implementation assumes: The input DFA uses `CharSet` as transition labels.
 * Acceptance is solely determined by inclusion of `originalNFAEndId` in the NFA state set.
 * The start state of the input DFA is `dfaStates[0]`.
 */
#include "dfa.h"
#include <map>
#include <algorithm>
#include <iostream>
#include <vector>
#include <set>
#include <tuple>

// 辅助：获取某个状态在哪个分区
int getPartitionId(int stateId, const std::vector<std::vector<int>>& partitions) {
    for (size_t i = 0; i < partitions.size(); ++i) {
        for (int state : partitions[i]) {
            if (state == stateId) return static_cast<int>(i);
        }
    }
    return -1;
}

// 辅助：获取状态在给定输入下的目标状态ID，如果没有则返回 -1
int getTargetState(int stateId, const CharSet& symbol, const std::vector<DFATransition>& transitions) {
    for (const auto& t : transitions) {
        if (t.fromStateId == stateId && t.transitionSymbol == symbol) {
            return t.toStateId;
        }
    }
    return -1;
}

void minimizeDFA(const std::vector<DFAState>& dfaStates,
                 const std::vector<DFATransition>& dfaTransitions,
                 int originalNFAEndId,
                 std::vector<DFAState>& minDfaStates,
                 std::vector<DFATransition>& minDfaTransitions) {
    
    if (dfaStates.empty()) return;

    // 创建状态ID到索引的映射
    std::map<int, int> stateIdToIdx;
    for (size_t i = 0; i < dfaStates.size(); ++i) {
        stateIdToIdx[dfaStates[i].id] = i;
    }

    // 1. 初始划分：使用 vector<vector<int>> 存储分区
    std::vector<std::vector<int>> partitions;
    std::vector<int> nonAcceptingStates;
    std::vector<int> acceptingStates;
    
    // 存储每个状态当前所在的分区编号
    std::vector<int> stateGroup(dfaStates.size());
    
    for (const auto& s : dfaStates) {
        if (s. nfaStates.count(originalNFAEndId)) {
            acceptingStates.push_back(s.id);
        } else {
            nonAcceptingStates.push_back(s. id);
        }
    }
    
    // 初始化分区和分组映射
    if (! nonAcceptingStates.empty()) {
        partitions.push_back(nonAcceptingStates);
        for (int state : nonAcceptingStates) {
            stateGroup[stateIdToIdx[state]] = 0;
        }
    }
    if (!acceptingStates.empty()) {
        int grpIdx = partitions.size();
        partitions.push_back(acceptingStates);
        for (int state : acceptingStates) {
            stateGroup[stateIdToIdx[state]] = grpIdx;
        }
    }

    // 收集所有出现过的输入符号
    std::vector<CharSet> alphabet;
    for (const auto& t : dfaTransitions) {
        bool exists = false;
        for (const auto& a : alphabet) {
            if (a == t.transitionSymbol) {
                exists = true;
                break;
            }
        }
        if (!exists) alphabet. push_back(t.transitionSymbol);
    }

    // 2. 不断分割分区（分区细化算法）
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<std::vector<int>> newPartitions;

        // 对每个分区进行检查
        for (size_t partIdx = 0; partIdx < partitions.size(); ++partIdx) {
            const auto& partition = partitions[partIdx];
            
            if (partition.size() <= 1) {
                newPartitions. push_back(partition);
                continue;
            }

            bool partitionSplit = false;
            
            // 对每个输入符号检查是否需要分割
            for (const auto& symbol : alphabet) {
                // 创建 transition signature -> states 的映射
                // 使用 vector<int> 作为签名，每个符号对应一个转移目标分组
                std::map<std::vector<int>, std::vector<int>> splitGroups;
                
                for (int stateId : partition) {
                    std::vector<int> signature;
                    
                    // 对所有符号计算转移签名
                    for (const auto& sym : alphabet) {
                        int target = getTargetState(stateId, sym, dfaTransitions);
                        int targetGroup = (target >= 0) ?  stateGroup[stateIdToIdx[target]] : -1;
                        signature.push_back(targetGroup);
                    }
                    
                    splitGroups[signature].push_back(stateId);
                }
                
                // 如果分割出多个子组，则进行分割
                if (splitGroups.size() > 1) {
                    changed = true;
                    partitionSplit = true;
                    
                    // 添加新分区
                    for (auto& entry : splitGroups) {
                        int newGrpIdx = newPartitions. size();
                        newPartitions.push_back(entry.second);
                        for (int state : entry.second) {
                            stateGroup[stateIdToIdx[state]] = newGrpIdx;
                        }
                    }
                    break; // 已分割，跳出符号循环
                }
            }
            
            // 如果没有分割，保持原样
            if (!partitionSplit) {
                newPartitions.push_back(partition);
            }
        }
        
        if (changed) {
            partitions = newPartitions;
            // 更新 stateGroup 映射
            for (size_t i = 0; i < partitions.size(); ++i) {
                for (int state : partitions[i]) {
                    stateGroup[stateIdToIdx[state]] = i;
                }
            }
        }
    }

    // 3.  构建最小化后的 DFA
    minDfaStates.clear();
    minDfaTransitions.clear();

    // 找到初始状态所在的分区
    int oldStartId = dfaStates[0].id;
    int startPartitionIdx = stateGroup[stateIdToIdx[oldStartId]];

    // 创建分区索引到新状态ID的映射
    std::map<int, int> partitionToNewId;
    partitionToNewId[startPartitionIdx] = 0; // 初始状态分区映射到 ID 0
    
    int nextId = 1;
    for (size_t i = 0; i < partitions.size(); ++i) {
        if (static_cast<int>(i) != startPartitionIdx) {
            partitionToNewId[i] = nextId++;
        }
    }

    // 创建旧状态ID到新状态ID的映射
    std::map<int, int> oldToNewMap;
    for (size_t i = 0; i < partitions.size(); ++i) {
        int newId = partitionToNewId[i];
        for (int oldId : partitions[i]) {
            oldToNewMap[oldId] = newId;
        }
    }

    // 创建新状态（按新ID排序）
    std::vector<std::pair<int, size_t>> idToPartitionIdx; // (newId, partitionIdx)
    for (size_t i = 0; i < partitions.size(); ++i) {
        idToPartitionIdx.push_back({partitionToNewId[i], i});
    }
    std::sort(idToPartitionIdx.begin(), idToPartitionIdx.end());

    // 构建新状态
    for (const auto& pair : idToPartitionIdx) {
        int newId = pair.first;
        size_t partIdx = pair.second;
        
        DFAState newState;
        newState.id = newId;
        newState.stateName = std::to_string(newId);
        
        // 检查是否为接受状态
        bool isAccepting = false;
        for (int oldId : partitions[partIdx]) {
            if (dfaStates[stateIdToIdx[oldId]].nfaStates.count(originalNFAEndId)) {
                isAccepting = true;
                break;
            }
        }
        
        if (isAccepting) {
            newState.nfaStates. insert(originalNFAEndId);
        }
        
        minDfaStates.push_back(newState);
    }

    // 4. 创建新转移
    std::set<std::tuple<int, int, CharSet>> addedTransitions; // 去重
    
    for (size_t i = 0; i < partitions.size(); ++i) {
        if (partitions[i].empty()) continue;
        
        // 取代表状态
        int representative = partitions[i][0];
        int fromNewId = oldToNewMap[representative];

        // 遍历所有输入符号
        for (const auto& symbol : alphabet) {
            int oldTarget = getTargetState(representative, symbol, dfaTransitions);
            if (oldTarget >= 0) {
                int toNewId = oldToNewMap[oldTarget];
                
                // 去重检查
                auto transKey = std::make_tuple(fromNewId, toNewId, symbol);
                if (addedTransitions.find(transKey) == addedTransitions.end()) {
                    minDfaTransitions.push_back({fromNewId, toNewId, symbol});
                    addedTransitions. insert(transKey);
                }
            }
        }
    }
}