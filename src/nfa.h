#ifndef NFA_H
#define NFA_H

#include <string>
#include <vector>
#include <iostream>

// ==============================
// 基础结构定义
// ==============================

struct Node {
    std::string nodeName;
    bool operator==(const Node& other) const {
        return nodeName == other.nodeName;
    }
};

struct Edge {
    Node startName;
    Node endName;
    std::string tranSymbol; // "" 表示 ε 转移
};

// NFA 单元
struct NFAUnit {
    std::vector<Edge> edges;
    Node start;
    Node end;
};

// ==============================
// NFA 构造函数声明
// ==============================

Node createNode();
NFAUnit createBasicElement(const std::string& symbol = "");
NFAUnit createUnion(const NFAUnit& left, const NFAUnit& right);
NFAUnit createConcat(const NFAUnit& left, const NFAUnit& right);
NFAUnit createStar(const NFAUnit& unit);

// NFA 可视化
void displayNFA(const NFAUnit& nfa);
void generateDotFile_NFA(const NFAUnit& nfa, const std::string& filename = "nfa.dot");

#endif // NFA_H