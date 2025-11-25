#ifndef NFA_H
#define NFA_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>

// ==============================
// 基础结构定义
// ==============================

// 实际存储节点数据的结构体
struct NodeImpl {
    int id; // 唯一整数 ID，用于核心逻辑和快速比较
    std::string debugName; // 仅用于调试和可视化
    
    // 构造函数
    NodeImpl(int id, std::string name) : id(id), debugName(std::move(name)) {}
};

// 使用 shared_ptr 管理节点生命周期
using Node = std::shared_ptr<NodeImpl>;

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