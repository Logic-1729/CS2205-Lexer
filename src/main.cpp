#include "lexer.h"
#include "regex_parser.h"
#include "nfa.h"
#include "dfa.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sys/stat.h>
#include <cstdlib>
#include <unistd.h>

// 函数声明
void runLexerMode();
void runSingleRegexMode(const std::string& outputDir);
void runPredefinedLexerMode();

// 辅助函数：转义 shell 特殊字符
std::string escapeShellArg(const std::string& arg) {
    std::string escaped;
    for (char c : arg) {
        // 转义 shell 特殊字符
        if (c == ' ' || c == '(' || c == ')' || c == '|' || c == '&' || 
            c == ';' || c == '<' || c == '>' || c == '*' || c == '?' || 
            c == '[' || c == ']' || c == '{' || c == '}' || c == '$' || 
            c == '`' || c == '\\' || c == '"' || c == '\'' || c == '! ') {
            escaped += '\\';
        }
        escaped += c;
    }
    return escaped;
}

// 辅助函数：检查命令是否存在（改进版）
bool commandExists(const std::string& command) {
    // 方法1：使用 which 命令
    std::string checkCmd = "which " + command + " > /dev/null 2>&1";
    if (system(checkCmd.c_str()) == 0) {
        return true;
    }
    
    // 方法2：尝试直接执行命令的 --version
    std::string versionCmd = command + " --version > /dev/null 2>&1";
    if (system(versionCmd. c_str()) == 0) {
        return true;
    }
    
    return false;
}

// 辅助函数：生成 PNG 图片（改进版）
bool generatePNG(const std::string& dotFile, const std::string& pngFile) {
    // 转义文件路径
    std::string escapedDot = escapeShellArg(dotFile);
    std::string escapedPng = escapeShellArg(pngFile);
    
    // 直接尝试执行，不再预先检查 dot 命令是否存在
    std::string cmd = "dot -Tpng " + escapedDot + " -o " + escapedPng + " 2>/dev/null";
    int result = system(cmd.c_str());
    
    // 如果命令执行成功并且文件确实生成了
    if (result == 0) {
        // 验证文件是否真的生成
        std::ifstream file(pngFile);
        return file.good();
    }
    
    return false;
}

// 辅助函数：规范化路径
std::string normalizePath(const std::string& path) {
    std::string result = path;
    
    // 移除开头和结尾的空格
    while (!result.empty() && std::isspace(static_cast<unsigned char>(result. front()))) {
        result. erase(result.begin());
    }
    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.back()))) {
        result.pop_back();
    }
    
    // 移除开头的 ". /"
    if (result.size() >= 2 && result[0] == '.' && result[1] == '/') {
        result = result.substr(2);
    }
    
    // 移除末尾的斜杠
    while (!result.empty() && (result.back() == '/' || result.back() == '\\')) {
        result.pop_back();
    }
    
    // 如果为空或只有"."，返回当前目录
    if (result.empty() || result == ".") {
        return ".";
    }
    
    return result;
}

// 辅助函数：拼接路径
std::string joinPath(const std::string& dir, const std::string& filename) {
    std::string normalizedDir = normalizePath(dir);
    
    // 如果是当前目录，直接返回文件名
    if (normalizedDir == ".") {
        return filename;
    }
    
    // 否则拼接目录和文件名
    return normalizedDir + "/" + filename;
}

// 辅助函数：确保目录存在
bool ensureDirectoryExists(const std::string& path) {
    std::string normalizedPath = normalizePath(path);
    
    // 当前目录总是存在
    if (normalizedPath == ".") {
        return true;
    }
    
    struct stat info;
    if (stat(normalizedPath.c_str(), &info) != 0) {
        // 目录不存在，尝试创建
        #ifdef _WIN32
            int result = _mkdir(normalizedPath.c_str());
        #else
            // 使用 system 调用 mkdir -p 来创建目录（支持特殊字符）
            std::string cmd = "mkdir -p " + escapeShellArg(normalizedPath) + " 2>/dev/null";
            int result = system(cmd. c_str());
        #endif
        
        if (result == 0) {
            std::cout << "Created directory: " << normalizedPath << std::endl;
            return true;
        }
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

int main(int argc, char* argv[]) {
    int choice = 0;
    std::string outputDir = ".";

    // 从命令行参数读取模式
    if (argc > 1) {
        try {
            choice = std::stoi(argv[1]);
        } catch (...) {
            choice = 0;
        }
    }

    // 从命令行参数读取输出目录
    if (argc > 2) {
        outputDir = argv[2];
    }

    // 如果没有指定模式，显示菜单
    if (choice == 0) {
        std::cout << "===========================================\n";
        std::cout << "  CS2205 Lexical Analyzer\n";
        std::cout << "===========================================\n\n";
        
        std::cout << "Select mode:\n";
        std::cout << "  1. Predefined Lexer (lang.l tokens)\n";
        std::cout << "  2. Custom Lexer (define your own tokens)\n";
        std::cout << "  3. Single Regex (Regex -> NFA -> DFA)\n";
        std::cout << "Enter choice (1-3): ";
        
        if (!(std::cin >> choice)) return 0;
        std::cin.ignore();
    }

    try {
        switch (choice) {
            case 1:
                runPredefinedLexerMode();
                break;
            case 2:
                runLexerMode();
                break;
            case 3:
                runSingleRegexMode(outputDir);
                break;
            default:
                std::cout << "Invalid choice.\n";
                return 1;
        }
    } catch (const RegexSyntaxError& e) {
        std::cerr << "\n[Syntax Error]: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n[Error]: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

void runPredefinedLexerMode() {
    std::cout << "\n=== Predefined Lexer Mode (lang.l) ===\n";
    
    Lexer lexer;
    lexer.initializeDefaultTokenClasses();
    
    std::cout << "\nBuilding lexer with " << lexer.getTokenClasses(). size() << " token types.. .\n";
    lexer. build();
    
    lexer.generateDotFile("lexer_dfa.dot");
    std::cout << "\nGenerated: lexer_dfa.dot\n";
    
    // 生成 PNG 图片
    std::cout << "\n=== Generating Visualization ===\n";
    if (generatePNG("lexer_dfa.dot", "lexer_dfa.png")) {
        std::cout << "✓ Generated: lexer_dfa.png\n";
    } else {
        std::cout << "⚠ Warning: Could not generate PNG.\n";
        std::cout << "  Please check if Graphviz is installed:\n";
        std::cout << "    Ubuntu/Debian: sudo apt-get install graphviz\n";
        std::cout << "    macOS:         brew install graphviz\n";
        std::cout << "  Or manually run: dot -Tpng lexer_dfa.dot -o lexer_dfa.png\n";
    }
    
    std::cout << "\n=== Tokenization ===\n";
    std::cout << "Enter code to analyze (or 'quit' to exit):\n";
    
    std::string input;
    while (true) {
        std::cout << "\n> ";
        if (! std::getline(std::cin, input)) break;
        if (input == "quit" || input == "exit") break;
        
        if (input.empty()) continue;
        
        try {
            auto tokens = lexer.tokenize(input);
            
            std::cout << "\nTokens:\n";
            std::cout << "┌──────┬────────┬──────────────────┬────────────────────────┐\n";
            std::cout << "│ Line │ Column │ Token Type       │ Lexeme                 │\n";
            std::cout << "├──────┼────────┼──────────────────┼────────────────────────┤\n";
            
            for (const auto& token : tokens) {
                std::cout << "│ " << std::setw(4) << token.line 
                         << " │ " << std::setw(6) << token.column
                         << " │ " << std::setw(16) << std::left << token.tokenClassName
                         << " │ " << std::setw(22) << ("\"" + token.lexeme + "\"")
                         << " │\n";
            }
            
            std::cout << "└──────┴────────┴──────────────────┴────────────────────────┘\n";
            std::cout << "Total: " << tokens.size() << " tokens\n";
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

void runLexerMode() {
    std::cout << "\n=== Custom Lexer Mode ===\n";
    
    Lexer lexer;
    
    std::cout << "Enter number of token classes: ";
    int n;
    if (!(std::cin >> n)) return;
    std::cin.ignore();
    
    for (int i = 0; i < n; ++i) {
        std::cout << "\nToken Class " << (i + 1) << ":\n";
        std::cout << "  Name: ";
        std::string name;
        std::getline(std::cin, name);
        
        std::cout << "  Regex: ";
        std::string regex;
        std::getline(std::cin, regex);
        
        lexer.addTokenClass(name, regex);
    }
    
    std::cout << "\nBuilding lexer with " << lexer.getTokenClasses().size() << " token types...\n";
    lexer.build();
    
    lexer.generateDotFile("custom_lexer_dfa.dot");
    std::cout << "\nGenerated: custom_lexer_dfa.dot\n";
    
    // 生成 PNG 图片
    std::cout << "\n=== Generating Visualization ===\n";
    if (generatePNG("custom_lexer_dfa. dot", "custom_lexer_dfa.png")) {
        std::cout << "✓ Generated: custom_lexer_dfa.png\n";
    } else {
        std::cout << "⚠ Warning: Could not generate PNG.\n";
        std::cout << "  Please check if Graphviz is installed:\n";
        std::cout << "    Ubuntu/Debian: sudo apt-get install graphviz\n";
        std::cout << "    macOS:         brew install graphviz\n";
        std::cout << "  Or manually run: dot -Tpng custom_lexer_dfa.dot -o custom_lexer_dfa.png\n";
    }
    
    std::cout << "\n=== Tokenization ===\n";
    std::cout << "Enter input to analyze (or 'quit' to exit):\n";
    
    std::string input;
    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, input)) break;
        if (input == "quit" || input == "exit") break;
        
        if (input.empty()) continue;
        
        try {
            auto tokens = lexer.tokenize(input);
            
            std::cout << "\nTokens:\n";
            std::cout << "┌──────┬────────┬──────────────────┬────────────────────────┐\n";
            std::cout << "│ Line │ Column │ Token Type       │ Lexeme                 │\n";
            std::cout << "├──────┼────────┼──────────────────┼────────────────────────┤\n";
            
            for (const auto& token : tokens) {
                std::cout << "│ " << std::setw(4) << token.line 
                         << " │ " << std::setw(6) << token.column
                         << " │ " << std::setw(16) << std::left << token.tokenClassName
                         << " │ " << std::setw(22) << ("\"" + token.lexeme + "\"")
                         << " │\n";
            }
            
            std::cout << "└──────┴────────┴──────────────────┴────────────────────────┘\n";
            std::cout << "Total: " << tokens.size() << " tokens\n";
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

void runSingleRegexMode(const std::string& outputDir) {
    // 规范化输出目录
    std::string normalizedDir = normalizePath(outputDir);
    
    std::cout << "\n=== Single Regex Mode ===\n";
    std::cout << "Output Directory: " << normalizedDir << "\n";
    
    // 确保输出目录存在
    if (!ensureDirectoryExists(normalizedDir)) {
        std::cerr << "ERROR: Could not create output directory: " << normalizedDir << "\n";
        return;
    }
    
    std::cout << "Enter regular expression: ";
    
    std::string regularExpression;
    if (!std::getline(std::cin, regularExpression)) return;

    try {
        // Step 1: 预处理
        auto tokens = preprocessRegex(regularExpression);
        auto tokensWithConcat = insertConcatSymbols(tokens);
        
        // Step 2: 转后缀
        InfixToPostfix converter(tokensWithConcat);
        converter.convert();
        const auto& postfix = converter.getPostfix();

        // Step 3: 构建 NFA
        NFAUnit nfa = regexToNFA(postfix);
        
        std::cout << "\n=== NFA ===" << std::endl;
        displayNFA(nfa);
        
        std::string nfaPath = joinPath(normalizedDir, "nfa_graph.dot");
        generateDotFile_NFA(nfa, nfaPath);
        std::cout << "Generated: " << nfaPath << std::endl;

        // Step 4: NFA 转 DFA
        std::vector<DFAState> dfaStates;
        std::vector<DFATransition> dfaTransitions;
        buildDFAFromNFA(nfa, dfaStates, dfaTransitions);
        int originalNFAEndId = nfa.end->id;

        std::cout << "\n=== Original DFA ===" << std::endl;
        displayDFA(dfaStates, dfaTransitions, originalNFAEndId);
        
        std::string dfaPath = joinPath(normalizedDir, "dfa_graph.dot");
        generateDotFile_DFA(dfaStates, dfaTransitions, originalNFAEndId, dfaPath);
        std::cout << "Generated: " << dfaPath << std::endl;

        // Step 5: DFA 最小化
        std::vector<DFAState> minDfaStates;
        std::vector<DFATransition> minDfaTransitions;
        minimizeDFA(dfaStates, dfaTransitions, originalNFAEndId, minDfaStates, minDfaTransitions);

        std::cout << "\n=== Minimized DFA ===" << std::endl;
        displayDFA(minDfaStates, minDfaTransitions, originalNFAEndId);
        
        std::string minDfaPath = joinPath(normalizedDir, "min_dfa_graph.dot");
        generateDotFile_DFA(minDfaStates, minDfaTransitions, originalNFAEndId, minDfaPath);
        std::cout << "Generated: " << minDfaPath << std::endl;
        
        // Step 6: 生成 PNG 图片
        std::cout << "\n=== Generating Visualizations ===\n";
        
        int pngCount = 0;
        
        std::string nfaPng = joinPath(normalizedDir, "nfa. png");
        if (generatePNG(nfaPath, nfaPng)) {
            std::cout << "✓ NFA:     " << nfaPng << "\n";
            pngCount++;
        } else {
            std::cout << "✗ Failed to generate NFA PNG\n";
        }
        
        std::string dfaPng = joinPath(normalizedDir, "dfa.png");
        if (generatePNG(dfaPath, dfaPng)) {
            std::cout << "✓ DFA:     " << dfaPng << "\n";
            pngCount++;
        } else {
            std::cout << "✗ Failed to generate DFA PNG\n";
        }
        
        std::string minDfaPng = joinPath(normalizedDir, "min_dfa.png");
        if (generatePNG(minDfaPath, minDfaPng)) {
            std::cout << "✓ Min-DFA: " << minDfaPng << "\n";
            pngCount++;
        } else {
            std::cout << "✗ Failed to generate Min-DFA PNG\n";
        }
        
        if (pngCount == 0) {
            std::cout << "\n⚠ No PNG files generated.\n";
            std::cout << "  Please check if Graphviz is installed:\n";
            std::cout << "    Ubuntu/Debian: sudo apt-get install graphviz\n";
            std::cout << "    macOS:         brew install graphviz\n";
            std::cout << "  Or manually run:\n";
            std::cout << "    cd " << escapeShellArg(normalizedDir) << "\n";
            std::cout << "    dot -Tpng nfa_graph.dot -o nfa.png\n";
            std::cout << "    dot -Tpng dfa_graph.dot -o dfa.png\n";
            std::cout << "    dot -Tpng min_dfa_graph.dot -o min_dfa.png\n";
        } else {
            std::cout << "\n✓ Generated " << pngCount << "/3 PNG files\n";
        }
        
        std::cout << "\n✓ All files saved to: " << normalizedDir << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during regex processing: " << e.what() << "\n";
        throw;
    }
}