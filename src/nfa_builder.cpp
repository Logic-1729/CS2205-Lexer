#include "nfa.h"
#include "regex_parser.h"
#include <stack>
#include <memory>
#include <algorithm>

// 全局节点计数器
static int globalNodeCounter = 0;

// 创建智能指针管理的节点
Node createNode() {
    int id = globalNodeCounter++;
    return std::make_shared<NodeImpl>(id, "q" + std::to_string(id));
}

// ... (其馀函数 createBasicElement, createUnion, createConcat, createStar, regexToNFA 保持不变)
// 注意：之前的 createConcat 使用了指针比较 (edge.startName == right.start)，这依然有效，因为 shared_ptr 比较的是地址。

NFAUnit createBasicElement(const std::string& symbol) {
    NFAUnit unit;
    unit.start = createNode();
    unit.end = createNode();
    unit.edges.push_back({unit.start, unit.end, symbol});
    return unit;
}

NFAUnit createUnion(const NFAUnit& left, const NFAUnit& right) {
    NFAUnit result;
    result.start = createNode();
    result.end = createNode();

    result.edges = left.edges;
    result.edges.insert(result.edges.end(), right.edges.begin(), right.edges.end());

    result.edges.push_back({result.start, left.start, ""});
    result.edges.push_back({result.start, right.start, ""});

    result.edges.push_back({left.end, result.end, ""});
    result.edges.push_back({right.end, result.end, ""});

    return result;
}

NFAUnit createConcat(const NFAUnit& left, const NFAUnit& right) {
    NFAUnit result = left; 
    
    std::vector<Edge> rightEdges = right.edges;
    for (auto& edge : rightEdges) {
        if (edge.startName == right.start) { 
            edge.startName = left.end; 
        }
        if (edge.endName == right.start) {
            edge.endName = left.end;   
        }
    }
    
    result.edges.insert(result.edges.end(), rightEdges.begin(), rightEdges.end());
    result.end = right.end;

    return result;
}

NFAUnit createStar(const NFAUnit& unit) {
    NFAUnit result;
    result.start = createNode();
    result.end = createNode();

    result.edges = unit.edges;

    result.edges.push_back({result.start, unit.start, ""});
    result.edges.push_back({unit.end, result.end, ""});
    result.edges.push_back({unit.end, unit.start, ""});
    result.edges.push_back({result.start, result.end, ""});

    return result;
}

NFAUnit regexToNFA(const std::vector<std::string>& postfix) {
    std::stack<NFAUnit> stk;

    for (const std::string& token : postfix) {
        if (token == "|") {
            if (stk.size() < 2) throw std::runtime_error("Invalid regex: | needs two operands");
            auto right = stk.top(); stk.pop();
            auto left = stk.top(); stk.pop();
            stk.push(createUnion(left, right));
        } else if (token == "+") {
            if (stk.size() < 2) throw std::runtime_error("Invalid regex: + needs two operands");
            auto right = stk.top(); stk.pop();
            auto left = stk.top(); stk.pop();
            stk.push(createConcat(left, right));
        } else if (token == "*") {
            if (stk.empty()) throw std::runtime_error("Invalid regex: * needs one operand");
            auto top = stk.top(); stk.pop();
            stk.push(createStar(top));
        } else {
            stk.push(createBasicElement(token));
        }
    }

    if (stk.size() != 1) {
        throw std::runtime_error("Invalid postfix expression");
    }

    std::cout << "Regex converted to NFA successfully!" << std::endl;
    return stk.top();
}