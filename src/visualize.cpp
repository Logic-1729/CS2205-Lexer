#include "nfa.h"
#include "dfa.h"
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>

std::string getDFAStateName(int id, const std::vector<DFAState>& dfaStates) {
    // 在最小化 DFA 中，ID 可能不连续或乱序，需要通过遍历查找
    for(const auto& s : dfaStates) {
        if (s.id == id) return s.stateName;
    }
    return "q" + std::to_string(id); // Fallback
}

// 辅助结构：用于聚合边的 Key
struct EdgeKey {
    int startId;
    int endId;
    bool operator<(const EdgeKey& other) const {
        if (startId != other.startId) return startId < other.startId;
        return endId < other.endId;
    }
};

// 辅助函数：聚合 DFA 边 (合并相同起终点的边)
std::map<EdgeKey, std::vector<std::string>> aggregateDFAEdges(const std::vector<DFATransition>& transitions) {
    std::map<EdgeKey, std::vector<std::string>> aggregated;
    for (const auto& t : transitions) {
        EdgeKey key{t.fromStateId, t.toStateId};
        aggregated[key].push_back(t.transitionSymbol.toString());
    }
    return aggregated;
}

// 辅助函数：聚合 NFA 边
std::map<EdgeKey, std::vector<std::string>> aggregateNFAEdges(const NFAUnit& nfa) {
    std::map<EdgeKey, std::vector<std::string>> aggregated;
    for (const auto& e : nfa.edges) {
        EdgeKey key{e.startName->id, e.endName->id};
        aggregated[key].push_back(e.symbol.toString());
    }
    return aggregated;
}

// 辅助函数：将多个标签合并为一个字符串 (e.g., "a,b")
std::string mergeLabels(const std::vector<std::string>& labels) {
    if (labels.empty()) return "";
    std::set<std::string> uniqueLabels(labels.begin(), labels.end());
    
    std::string result = "";
    bool first = true;
    for (const auto& label : uniqueLabels) {
        if (!first) result += ",";
        result += label;
        first = false;
    }
    return result;
}

void displayNFA(const NFAUnit& nfa) {
    std::cout << "NFA States (Start: " << nfa.start->debugName 
              << ", End: " << nfa.end->debugName << ")\nTransitions:\n";
    auto aggregated = aggregateNFAEdges(nfa);
    for (const auto& item : aggregated) {
        std::string label = mergeLabels(item.second);
        std::cout << "  q" << item.first.startId << " --(" << label << ")--> q" << item.first.endId << "\n";
    }
    std::cout << "End of NFA\n" << std::endl;
}

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                int originalNFAEndId) {
    std::cout << "States:\n";
    for (const auto& state : dfaStates) {
        std::cout << "State " << state.stateName;
        if (state.nfaStates.count(originalNFAEndId)) std::cout << " [Accepting]";
        std::cout << "\n";
    }
    std::cout << "Transitions:\n";
    auto aggregated = aggregateDFAEdges(dfaTransitions);
    for (const auto& item : aggregated) {
        std::string fromName = getDFAStateName(item.first.startId, dfaStates);
        std::string toName = getDFAStateName(item.first.endId, dfaStates);
        std::string label = mergeLabels(item.second);
        std::cout << "  " << fromName << " --(" << label << ")--> " << toName << "\n";
    }
}

void generateDotFile_NFA(const NFAUnit& nfa, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) return;
    file << "digraph NFA { rankdir=LR; node [shape=circle];\n";
    file << "  " << nfa.end->debugName << " [shape=doublecircle];\n";
    file << "  __start0 [shape=none, label=\"\"]; __start0 -> " << nfa.start->debugName << ";\n";
    
    auto aggregated = aggregateNFAEdges(nfa);
    // ID to Name mapping
    std::map<int, std::string> idToName;
    idToName[nfa.start->id] = nfa.start->debugName;
    idToName[nfa.end->id] = nfa.end->debugName;
    for(const auto& e : nfa.edges) {
        idToName[e.startName->id] = e.startName->debugName;
        idToName[e.endName->id] = e.endName->debugName;
    }

    for (const auto& item : aggregated) {
        std::string startName = idToName.count(item.first.startId) ? idToName[item.first.startId] : ("q" + std::to_string(item.first.startId));
        std::string endName = idToName.count(item.first.endId) ? idToName[item.first.endId] : ("q" + std::to_string(item.first.endId));
        std::string label = mergeLabels(item.second);
        
        file << "  " << startName << " -> " << endName 
             << " [label=\"" << label << "\"];\n";
    }
    file << "}\n";
    file.close();
}

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         int originalNFAEndId,
                         const std::string& filename) {
    std::ofstream file(filename);
    if (!file) return;
    file << "digraph DFA { rankdir=LR; node [shape=circle];\n";
    
    if (!dfaStates.empty()) {
        // 默认第一个状态是初始状态
        file << "  __start0 [shape=none, label=\"\"];\n";
        file << "  __start0 -> " << dfaStates[0].stateName << ";\n";
    }

    for (const auto& state : dfaStates) {
        if (state.nfaStates.count(originalNFAEndId))
            file << "  " << state.stateName << " [shape=doublecircle];\n";
    }
    
    auto aggregated = aggregateDFAEdges(dfaTransitions);
    for (const auto& item : aggregated) {
        std::string fromName = getDFAStateName(item.first.startId, dfaStates);
        std::string toName = getDFAStateName(item.first.endId, dfaStates);
        std::string label = mergeLabels(item.second);
        
        file << "  " << fromName << " -> " << toName 
             << " [label=\"" << label << "\"];\n";
    }
    file << "}\n";
    file.close();
}