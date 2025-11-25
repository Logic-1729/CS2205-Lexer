#include "nfa_dfa_builder.h"
#include <memory>

// 全局节点计数器（线程不安全，仅用于演示）
static int globalNodeCounter = 0;

Node createNode() {
    return {"q" + std::to_string(globalNodeCounter++)};
}

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

    // Add all edges from left and right
    result.edges = left.edges;
    result.edges.insert(result.edges.end(), right.edges.begin(), right.edges.end());

    // ε-transitions
    result.edges.push_back({result.start, left.start, ""});
    result.edges.push_back({result.start, right.start, ""});
    result.edges.push_back({left.end, result.end, ""});
    result.edges.push_back({right.end, result.end, ""});

    return result;
}

NFAUnit createConcat(const NFAUnit& left, const NFAUnit& right) {
    NFAUnit result;
    result.start = left.start;
    result.end = right.end;

    // Merge edges
    result.edges = left.edges;
    result.edges.insert(result.edges.end(), right.edges.begin(), right.edges.end());

    // Connect left.end -> right.start with ε
    result.edges.push_back({left.end, right.start, ""});

    return result;
}

NFAUnit createStar(const NFAUnit& unit) {
    NFAUnit result;
    result.start = createNode();
    result.end = createNode();

    result.edges = unit.edges;

    // ε from new start to new end (for ε)
    result.edges.push_back({result.start, result.end, ""});
    // ε from new start to unit start
    result.edges.push_back({result.start, unit.start, ""});
    // ε from unit end to unit start (loop)
    result.edges.push_back({unit.end, unit.start, ""});
    // ε from unit end to new end
    result.edges.push_back({unit.end, result.end, ""});

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
            // Literal symbol (including char sets like "abc")
            stk.push(createBasicElement(token));
        }
    }

    if (stk.size() != 1) {
        throw std::runtime_error("Invalid postfix expression");
    }

    std::cout << "Regex converted to NFA successfully!" << std::endl;
    return stk.top();
}