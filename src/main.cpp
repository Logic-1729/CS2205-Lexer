#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> // for system()

int main(int argc, char* argv[]) {
    std::string outputDir = ".";
    if (argc > 1) {
        outputDir = argv[1];
    }

    // 修复：使用 \"./path\" 格式，防止路径以 - 开头导致 mkdir 报错
    // 注意：如果 outputDir 已经包含 "./" 前缀（脚本中已处理），这里重复加也没问题
    std::string mkdirCmd;
    if (outputDir[0] == '-') {
        mkdirCmd = "mkdir -p \"./" + outputDir + "\"";
    } else {
        mkdirCmd = "mkdir -p \"" + outputDir + "\"";
    }
    
    // 消除未使用返回值的警告
    if (system(mkdirCmd.c_str()) != 0) {
        std::cerr << "Warning: Failed to create directory '" << outputDir << "'" << std::endl;
        // 不退出，尝试继续运行（可能目录已存在或有写入权限）
    }

    std::string regularExpression;
    // 读取输入
    if (!(std::cin >> regularExpression)) return 0;

    try {
        // Step 1: 预处理
        std::vector<Token> tokens = preprocessRegex(regularExpression);

        // Step 2: 插入连接符
        std::vector<Token> tokensWithConcat = insertConcatSymbols(tokens);

        // Step 3: 中缀转后缀
        InfixToPostfix converter(tokensWithConcat);
        converter.convert();
        const std::vector<Token>& postfix = converter.getPostfix();

        // Step 4: 后缀转 NFA
        NFAUnit nfa = regexToNFA(postfix);

        // Step 5: 可视化 NFA
        displayNFA(nfa);
        // 确保路径连接正确
        std::string nfaPath = outputDir + "/nfa_graph.dot";
        generateDotFile_NFA(nfa, nfaPath);
        std::cout << "Generated: " << nfaPath << std::endl;

        // Step 6: NFA 转 DFA
        std::vector<DFAState> dfaStates;
        std::vector<DFATransition> dfaTransitions;
        buildDFAFromNFA(nfa, dfaStates, dfaTransitions);

        // Step 7: 标记接受状态
        int originalNFAEndId = nfa.end->id;

        // Step 8: 可视化 DFA
        displayDFA(dfaStates, dfaTransitions, originalNFAEndId);
        std::string dfaPath = outputDir + "/dfa_graph.dot";
        generateDotFile_DFA(dfaStates, dfaTransitions, originalNFAEndId, dfaPath);
        std::cout << "Generated: " << dfaPath << std::endl;

        std::cout << "流程完成！\n";
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}