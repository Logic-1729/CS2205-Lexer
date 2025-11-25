#ifndef NFA_DFA_BUILDER_H
#define NFA_DFA_BUILDER_H

#include <iostream>
#include <fstream>
#include <cctype>
#include <stack>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>

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

// NFA 单元：支持动态边数量
struct NFAUnit {
    std::vector<Edge> edges;
    Node start;
    Node end;
};

// DFA 状态：由一组 NFA 状态（节点名）构成
struct DFAState {
    std::set<std::string> nfaStates;
    std::string stateName;

    bool operator==(const DFAState& other) const {
        return nfaStates == other.nfaStates;
    }

    bool operator<(const DFAState& other) const {
        return nfaStates < other.nfaStates;
    }
};

// DFA 转移
struct DFATransition {
    DFAState fromState;
    DFAState toState;
    std::string transitionSymbol;
};

// ==============================
// NFA 构造函数声明
// ==============================

Node createNode();

// 基本单元：单个符号（或 ε）
NFAUnit createBasicElement(const std::string& symbol = "");

// 选择：a | b
NFAUnit createUnion(const NFAUnit& left, const NFAUnit& right);

// 连接：ab
NFAUnit createConcat(const NFAUnit& left, const NFAUnit& right);

// 克林闭包：a*
NFAUnit createStar(const NFAUnit& unit);

// 拷贝 NFA 单元（用于避免引用冲突）
NFAUnit copyNFA(const NFAUnit& src);

// ==============================
// 正则表达式预处理与解析
// ==============================

std::vector<std::string> preprocessRegex(const std::string& re);
std::vector<std::string> insertConcatSymbols(const std::vector<std::string>& tokens);

class InfixToPostfix {
public:
    explicit InfixToPostfix(const std::vector<std::string>& infix);
    void convert();
    const std::vector<std::string>& getPostfix() const;

private:
    static bool isLetter(const std::string& token);
    static int getISP(char op);
    static int getICP(char op);

    std::vector<std::string> infix_;
    std::vector<std::string> postfix_;
};

NFAUnit regexToNFA(const std::vector<std::string>& postfix);

// ==============================
// NFA → DFA 转换
// ==============================

// ε-闭包：输入一组状态名，返回闭包（DFAState 形式）
DFAState epsilonClosure(const std::set<std::string>& states, const NFAUnit& nfa);

// move 函数：给定 DFA 状态和符号，返回 move 后的 ε-闭包
DFAState move(const DFAState& state, const std::string& symbol, const NFAUnit& nfa);

// 工具函数
bool isDFAStateInVector(const std::vector<DFAState>& dfaStates, const DFAState& target);
bool isTransitionInVector(const DFAState& from, const DFAState& to,
                          const std::string& symbol,
                          const std::vector<DFATransition>& transitions);

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions);

// ==============================
// 输出与可视化
// ==============================

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions);

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         const std::string& filename = "dfa.dot");

void generateDotFile_NFA(const NFAUnit& nfa, const std::string& filename = "nfa.dot");

void displayNFA(const NFAUnit& nfa);

// ==============================
// 辅助函数
// ==============================

bool isLetter(const std::string& s);

#endif // NFA_DFA_BUILDER_H