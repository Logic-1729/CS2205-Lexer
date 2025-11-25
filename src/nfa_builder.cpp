#include "nfa.h"
#include "regex_parser.h"
#include <stack>
#include <memory>
#include <algorithm>

static int globalNodeCounter = 0;

Node createNode() {
    int id = globalNodeCounter++;
    return std::make_shared<NodeImpl>(id, "q" + std::to_string(id));
}

NFAUnit createBasicElement(const CharSet& symbol) {
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
    result.edges.push_back({result.start, left.start, CharSet()}); 
    result.edges.push_back({result.start, right.start, CharSet()});
    result.edges.push_back({left.end, result.end, CharSet()});
    result.edges.push_back({right.end, result.end, CharSet()});
    return result;
}

NFAUnit createConcat(const NFAUnit& left, const NFAUnit& right) {
    NFAUnit result = left; 
    std::vector<Edge> rightEdges = right.edges;
    for (auto& edge : rightEdges) {
        if (edge.startName == right.start) edge.startName = left.end; 
        if (edge.endName == right.start) edge.endName = left.end;   
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
    result.edges.push_back({result.start, unit.start, CharSet()});
    result.edges.push_back({unit.end, result.end, CharSet()});
    result.edges.push_back({unit.end, unit.start, CharSet()});
    result.edges.push_back({result.start, result.end, CharSet()});
    return result;
}

NFAUnit createQuestion(const NFAUnit& unit) {
    NFAUnit result;
    result.start = createNode();
    result.end = createNode();
    result.edges = unit.edges;
    result.edges.push_back({result.start, unit.start, CharSet()});
    result.edges.push_back({unit.end, result.end, CharSet()});
    result.edges.push_back({result.start, result.end, CharSet()});
    return result;
}

NFAUnit createPlus(const NFAUnit& unit) {
    NFAUnit result;
    result.start = createNode();
    result.end = createNode();
    result.edges = unit.edges;
    result.edges.push_back({result.start, unit.start, CharSet()});
    result.edges.push_back({unit.end, result.end, CharSet()});
    result.edges.push_back({unit.end, unit.start, CharSet()});
    return result;
}

NFAUnit regexToNFA(const std::vector<Token>& postfix) {
    std::stack<NFAUnit> stk;

    for (const Token& token : postfix) {
        if (token.isOperator()) {
            if (token.opVal == '|') {
                auto right = stk.top(); stk.pop();
                auto left = stk.top(); stk.pop();
                stk.push(createUnion(left, right));
            } else if (token.opVal == EXPLICIT_CONCAT_OP) { // 使用全局常量匹配
                auto right = stk.top(); stk.pop();
                auto left = stk.top(); stk.pop();
                stk.push(createConcat(left, right));
            } else if (token.opVal == '*') {
                auto top = stk.top(); stk.pop();
                stk.push(createStar(top));
            } else if (token.opVal == '?') {
                auto top = stk.top(); stk.pop();
                stk.push(createQuestion(top));
            } else if (token.opVal == '+') {
                auto top = stk.top(); stk.pop();
                stk.push(createPlus(top));
            }
        } else {
            stk.push(createBasicElement(token.operandVal));
        }
    }

    if (stk.size() != 1) throw std::runtime_error("Invalid postfix expression");
    std::cout << "Regex converted to NFA successfully!" << std::endl;
    return stk.top();
}