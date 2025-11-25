#!/bin/bash

# 确保脚本抛出错误时停止执行
set -e

echo "=== 正在编译 Regex Automata 项目 ==="

# 编译命令
g++ -std=c++17 -O2 \
    src/main.cpp \
    src/regex_preprocessor.cpp \
    src/infix_to_postfix.cpp \
    src/nfa_builder.cpp \
    src/dfa_converter.cpp \
    src/visualize.cpp \
    -o regex_automata

echo "=== 编译成功！==="
echo "生成的执行文件为: ./regex_automata"
echo ""

# 运行程序
echo "请在下方输入正则表达式（例如: [_a-zA-Z][_a-zA-Z0-9]*）:"
./regex_automata

echo ""
echo "=== 正在生成可视化图片 ==="

if command -v dot &> /dev/null; then
    if [ -f "nfa_graph.dot" ]; then
        echo "生成 NFA 图片: nfa.png"
        dot -Tpng nfa_graph.dot -o nfa.png
    else
        echo "警告: 未找到 nfa_graph.dot"
    fi

    if [ -f "dfa_graph.dot" ]; then
        echo "生成 DFA 图片: dfa.png"
        dot -Tpng dfa_graph.dot -o dfa.png
    else
        echo "警告: 未找到 dfa_graph.dot"
    fi
else
    echo "错误: 未找到 'dot' 命令。请安装 Graphviz 以生成 PNG 图片。"
fi

echo "=== 全部完成 ==="