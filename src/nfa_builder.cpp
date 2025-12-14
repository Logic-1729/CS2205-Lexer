/*
 * nfa_builder.cpp - implements Thompson's construction algorithm to build an NFA from
 * a postfix regular expression token sequence. It provides functions to construct NFA fragments
 * for basic symbols and regex operators, and combines them according to the postfix order.
 * It features:
 * - createBasicElement: creates an NFA for a single `CharSet` (including character ranges).
 * - Operator handlers:
 *   * createUnion for '|'
 *   * createConcat for explicit concatenation
 *   * createStar for '*'
 *   * createQuestion for '?' (0 or 1)
 *   * createPlus for '+' (1 or more)
 * - All NFAs use epsilon transitions (represented by default-constructed `CharSet`) for
 * control flow (e.g., branching in union, looping in star).
 * - createConcat: performs node merging by redirecting edges that reference the right
 * operand's start node to the left operand's end node, avoiding unnecessary epsilon transitions.
 * - regexToNFA: uses a stack to process the postfix token stream, applying operator logic
 * and operand construction, and validates stack state for correctness.
 * - globalNodeCounter: ensures unique node IDs across the entire NFA.
 */
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
            // 双目操作符
            if (token.opVal == '|' || token.opVal == EXPLICIT_CONCAT_OP) {
                if (stk.size() < 2) throw RegexSyntaxError("Missing operands for operator '" + std::string(1, token.opVal) + "'.");
                auto right = stk.top(); stk.pop();
                auto left = stk.top(); stk.pop();
                
                if (token.opVal == '|') stk.push(createUnion(left, right));
                else stk.push(createConcat(left, right));
            } 
            // 单目操作符
            else if (token.opVal == '*' || token.opVal == '?' || token.opVal == '+') {
                if (stk.empty()) throw RegexSyntaxError("Missing operand for operator '" + std::string(1, token.opVal) + "'.");
                auto top = stk.top(); stk.pop();
                
                if (token.opVal == '*') stk.push(createStar(top));
                else if (token.opVal == '?') stk.push(createQuestion(top));
                else stk.push(createPlus(top));
            }
        } else {
            stk.push(createBasicElement(token.operandVal));
        }
    }

    if (stk.size() != 1) throw RegexSyntaxError("Invalid regex: Resulting NFA stack has " + std::to_string(stk.size()) + " elements (should be 1). Check for unbalanced operators.");
    
    std::cout << "Regex converted to NFA successfully!" << std::endl;
    return stk.top();
}