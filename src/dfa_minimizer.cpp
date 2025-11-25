#include "dfa.h"
#include <map>
#include <algorithm>
#include <iostream>

// 辅助：获取某个状态在哪个分区
int getPartitionId(int stateId, const std::vector<std::set<int>>& partitions) {
    for (size_t i = 0; i < partitions.size(); ++i) {
        if (partitions[i].count(stateId)) return static_cast<int>(i);
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

    // 1. 初始划分：接受状态组 和 非接受状态组
    std::set<int> accepting, nonAccepting;
    for (const auto& s : dfaStates) {
        if (s.nfaStates.count(originalNFAEndId)) {
            accepting.insert(s.id);
        } else {
            nonAccepting.insert(s.id);
        }
    }

    std::vector<std::set<int>> partitions;
    if (!nonAccepting.empty()) partitions.push_back(nonAccepting);
    if (!accepting.empty()) partitions.push_back(accepting);

    // 收集所有出现过的输入符号
    std::vector<CharSet> alphabet;
    for (const auto& t : dfaTransitions) {
        bool exists = false;
        for(const auto& a : alphabet) if(a == t.transitionSymbol) exists = true;
        if(!exists) alphabet.push_back(t.transitionSymbol);
    }

    // 2. 不断分割分区
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<std::set<int>> newPartitions;

        for (const auto& group : partitions) {
            if (group.size() <= 1) {
                newPartitions.push_back(group);
                continue;
            }

            // 尝试根据每个输入符号分割当前组
            // Map: Signature (vector of target partition IDs) -> Subgroup
            std::map<std::vector<int>, std::set<int>> splitGroups;

            for (int stateId : group) {
                std::vector<int> signature;
                for (const auto& symbol : alphabet) {
                    int target = getTargetState(stateId, symbol, dfaTransitions);
                    if (target == -1) {
                        signature.push_back(-1); // 死状态/无转移
                    } else {
                        signature.push_back(getPartitionId(target, partitions));
                    }
                }
                splitGroups[signature].insert(stateId);
            }

            if (splitGroups.size() > 1) {
                changed = true;
            }
            for (const auto& entry : splitGroups) {
                newPartitions.push_back(entry.second);
            }
        }
        partitions = newPartitions;
    }

    // 3. 构建最小化后的 DFA
    minDfaStates.clear();
    minDfaTransitions.clear();

    // Map: Old State ID -> New Partition ID
    std::map<int, int> oldToNewMap;
    
    // 创建新状态
    for (size_t i = 0; i < partitions.size(); ++i) {
        DFAState newState;
        newState.id = static_cast<int>(i);
        newState.stateName = std::to_string(i);
        
        // 确定是否为接受状态（只要分区里有一个原接受状态，该分区即为接受状态）
        bool isAccepting = false;
        for (int oldId : partitions[i]) {
            oldToNewMap[oldId] = newState.id;
            // 检查 oldId 是否是原接受状态
            // 这里稍微低效一点，重新查一遍原状态列表
            for(const auto& s : dfaStates) {
                if(s.id == oldId && s.nfaStates.count(originalNFAEndId)) {
                    isAccepting = true; 
                    break;
                }
            }
        }
        
        if (isAccepting) {
            // 标记新状态包含 NFA 终态 ID，以便 visualize.cpp 识别
            newState.nfaStates.insert(originalNFAEndId);
        }
        
        minDfaStates.push_back(newState);
    }

    // 确定初始状态
    // 假设 dfaStates[0] 是初始状态
    int oldStartId = dfaStates[0].id;
    int newStartId = oldToNewMap[oldStartId];
    
    // 确保最小化 DFA 的初始状态在 vector 的第 0 位
    if (newStartId != 0) {
        std::swap(minDfaStates[0], minDfaStates[newStartId]);
        // 更新映射关系（简单的 ID 交换可能不够，这里仅交换 vector 位置）
        // 为了保持 ID 一致性，最好在生成 newState 时就处理，
        // 但最简单的方法是后续绘图时指明 start，或者在这里做一个重新编号。
        // 为了简单，我们让 generateDotFile 总是把 vector[0] 当作 start。
        // 需要修正 minDfaStates[0].id 
        int tempId = minDfaStates[0].id;
        minDfaStates[0].id = minDfaStates[newStartId].id;
        minDfaStates[newStartId].id = tempId;
        
        // 重新建立 oldToNewMap 指向正确的 vector index
        for(size_t i=0; i<partitions.size(); ++i) {
            for(int old : partitions[i]) {
                if (static_cast<int>(i) == newStartId) oldToNewMap[old] = 0;
                else if (static_cast<int>(i) == 0) oldToNewMap[old] = newStartId;
                else oldToNewMap[old] = static_cast<int>(i);
            }
        }
    }

    // 创建新转移
    // 对于每个分区，取出一个代表状态，查看其转移
    for (size_t i = 0; i < partitions.size(); ++i) {
        if (partitions[i].empty()) continue;
        int representative = *partitions[i].begin();
        int fromNewId = oldToNewMap[representative]; // 应该是 i 或被交换过的值

        // 遍历字母表检查转移
        for (const auto& symbol : alphabet) {
            int oldTarget = getTargetState(representative, symbol, dfaTransitions);
            if (oldTarget != -1) {
                int toNewId = oldToNewMap[oldTarget];
                
                // 检查是否已经存在相同的转移（避免重复）
                bool exists = false;
                for(const auto& t : minDfaTransitions) {
                    if(t.fromStateId == fromNewId && t.toStateId == toNewId && t.transitionSymbol == symbol) {
                        exists = true; break;
                    }
                }
                if(!exists) {
                    minDfaTransitions.push_back({fromNewId, toNewId, symbol});
                }
            }
        }
    }
}