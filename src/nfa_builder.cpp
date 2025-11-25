#include "nfa.h"
#include "regex_parser.h"
#include <stack>
#include <memory>
#include <algorithm>

// 全局节点计数器
static int globalNodeCounter = 0;

Node createNode() {
    return {"q" + std::to_string(globalNodeCounter++)};
}

NFAUnit createBasicElement(const std::string& symbol) {
    NFAUnit unit;
    unit.start = createNode();
    unit.end = createNode();
    // 创建一条边 start -> end
    unit.edges.push_back({unit.start, unit.end, symbol});
    return unit;
}

NFAUnit createUnion(const NFAUnit& left, const NFAUnit& right) {
    NFAUnit result;
    result.start = createNode();
    result.end = createNode();

    // 复制所有边
    result.edges = left.edges;
    result.edges.insert(result.edges.end(), right.edges.begin(), right.edges.end());

    // 新起点通过 ε 到达 left 和 right 的起点
    result.edges.push_back({result.start, left.start, ""});
    result.edges.push_back({result.start, right.start, ""});

    // left 和 right 的终点通过 ε 到达新终点
    result.edges.push_back({left.end, result.end, ""});
    result.edges.push_back({right.end, result.end, ""});

    return result;
}

// 优化：连接操作不引入新节点，而是合并节点
// 参考 reference 代码中的 act_join
NFAUnit createConcat(const NFAUnit& left, const NFAUnit& right) {
    NFAUnit result = left; // 从左侧开始
    
    // 我们需要将 right 中的所有边加入 result
    // 但是，right 中所有从 right.start 出发的边，现在应该从 left.end 出发
    // 实际上就是把 right.start 这个节点替换为 left.end
    
    std::vector<Edge> rightEdges = right.edges;
    for (auto& edge : rightEdges) {
        if (edge.startName.nodeName == right.start.nodeName) {
            edge.startName = left.end;
        }
        if (edge.endName.nodeName == right.start.nodeName) {
            edge.endName = left.end;
        }
    }
    
    // 将调整后的 right 边加入 result
    result.edges.insert(result.edges.end(), rightEdges.begin(), rightEdges.end());
    
    // 新的终点是 right 的终点
    result.end = right.end;

    return result;
}

NFAUnit createStar(const NFAUnit& unit) {
    NFAUnit result;
    result.start = createNode();
    result.end = createNode();

    result.edges = unit.edges;

    // ε from new start to unit start
    result.edges.push_back({result.start, unit.start, ""});
    
    // ε from unit end to new end
    result.edges.push_back({unit.end, result.end, ""});
    
    // ε from unit end to unit start (loop)
    result.edges.push_back({unit.end, unit.start, ""});
    
    // ε from new start to new end (skip, 匹配空串)
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
            // Literal symbol (including char sets like "[a-z]")
            stk.push(createBasicElement(token));
        }
    }

    if (stk.size() != 1) {
        throw std::runtime_error("Invalid postfix expression");
    }

    std::cout << "Regex converted to NFA successfully!" << std::endl;
    return stk.top();
}