#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <set>
#include <algorithm>

// ==============================
// 字符集与区间定义
// ==============================

// 表示一个字符区间 [start, end]
struct CharRange {
    char start;
    char end;

    bool operator<(const CharRange& other) const {
        if (start != other.start) return start < other.start;
        return end < other.end;
    }
    
    bool operator==(const CharRange& other) const {
        return start == other.start && end == other.end;
    }
};

// 字符集类，支持添加区间、单个字符
class CharSet {
public:
    std::set<CharRange> ranges;
    bool isEpsilon; // 是否为 epsilon 边

    CharSet() : isEpsilon(true) {} // 默认是 epsilon
    CharSet(char c) : isEpsilon(false) { addRange(c, c); }
    CharSet(char start, char end) : isEpsilon(false) { addRange(start, end); }

    void addRange(char start, char end) {
        if (start > end) return;
        ranges.insert({start, end});
        isEpsilon = false;
    }

    // 检查字符是否在集合中
    bool match(char c) const {
        if (isEpsilon) return false;
        for (const auto& r : ranges) {
            if (c >= r.start && c <= r.end) return true;
        }
        return false;
    }

    // 转换为字符串用于显示
    std::string toString() const {
        if (isEpsilon) return "ε";
        std::string res;
        if (ranges.size() > 1 || (ranges.size() == 1 && ranges.begin()->start != ranges.begin()->end)) {
            res += "[";
            for (const auto& r : ranges) {
                res += r.start;
                if (r.start != r.end) {
                    res += "-";
                    res += r.end;
                }
            }
            res += "]";
        } else if (!ranges.empty()) {
            res += ranges.begin()->start;
        }
        return res;
    }
    
    // 用于 map key 的比较
    bool operator<(const CharSet& other) const {
        if (isEpsilon != other.isEpsilon) return isEpsilon < other.isEpsilon;
        return ranges < other.ranges;
    }
    
    bool operator==(const CharSet& other) const {
        return isEpsilon == other.isEpsilon && ranges == other.ranges;
    }
};

// ==============================
// 基础结构定义
// ==============================

struct NodeImpl {
    int id;
    std::string debugName;
    NodeImpl(int id, std::string name) : id(id), debugName(std::move(name)) {}
};

using Node = std::shared_ptr<NodeImpl>;

struct Edge {
    Node startName; 
    Node endName;   
    CharSet symbol; // 替换原来的 string tranSymbol
};

struct NFAUnit {
    std::vector<Edge> edges;
    Node start;
    Node end;
};

// ==============================
// NFA 构造函数声明
// ==============================

Node createNode();
NFAUnit createBasicElement(const CharSet& symbol);
NFAUnit createUnion(const NFAUnit& left, const NFAUnit& right);
NFAUnit createConcat(const NFAUnit& left, const NFAUnit& right);
NFAUnit createStar(const NFAUnit& unit);
// 新增语法糖支持
NFAUnit createQuestion(const NFAUnit& unit); // ? (0 or 1)
NFAUnit createPlus(const NFAUnit& unit);     // + (1 or more)

void displayNFA(const NFAUnit& nfa);
void generateDotFile_NFA(const NFAUnit& nfa, const std::string& filename = "nfa.dot");