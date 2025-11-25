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

// NFA 单元
struct NFAUnit {
    std::vector<Edge> edges;
    Node start;
    Node end;
};

// DFA 状态
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
NFAUnit createBasicElement(const std::string& symbol = "");
NFAUnit createUnion(const NFAUnit& left, const NFAUnit& right);
NFAUnit createConcat(const NFAUnit& left, const NFAUnit& right);
NFAUnit createStar(const NFAUnit& unit);

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
    std::vector<std::string> infix_;
    std::vector<std::string> postfix_;
    bool isLetter(const std::string& token);
    int getISP(char op);
    int getICP(char op);
};

NFAUnit regexToNFA(const std::vector<std::string>& postfix);

// ==============================
// NFA → DFA 转换
// ==============================

DFAState epsilonClosure(const std::set<std::string>& states, const NFAUnit& nfa);
DFAState move(const DFAState& state, const std::string& symbol, const NFAUnit& nfa);

void buildDFAFromNFA(const NFAUnit& nfa,
                     std::vector<DFAState>& dfaStates,
                     std::vector<DFATransition>& dfaTransitions);

// ==============================
// 输出与可视化
// ==============================

void displayDFA(const std::vector<DFAState>& dfaStates,
                const std::vector<DFATransition>& dfaTransitions,
                const std::string& originalNFAEnd);

void generateDotFile_DFA(const std::vector<DFAState>& dfaStates,
                         const std::vector<DFATransition>& dfaTransitions,
                         const std::string& originalNFAEnd,
                         const std::string& filename = "dfa.dot");

void generateDotFile_NFA(const NFAUnit& nfa, const std::string& filename = "nfa.dot");

void displayNFA(const NFAUnit& nfa);

// ==============================
// 辅助函数
// ==============================

bool isLetter(const std::string& s);

#endif // NFA_DFA_BUILDER_H