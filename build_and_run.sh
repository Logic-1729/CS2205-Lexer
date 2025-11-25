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

# 1. 询问输入的正则表达式个数
read -p "请输入要处理的正则表达式个数: " count

# 校验输入是否为数字
if ! [[ "$count" =~ ^[0-9]+$ ]]; then
    echo "错误: 请输入有效的数字。"
    exit 1
fi

# 循环处理
for ((i=1; i<=count; i++))
do
    echo "------------------------------------------------"
    echo "正在处理第 $i 个正则表达式 (共 $count 个)"
    
    # 读取正则表达式
    read -p "请输入正则表达式: " regex_input
    
    # 2. 设置文件夹名
    # 只替换斜杠 /，因为它是路径分隔符，无法作为文件名的一部分
    # 其他字符如 | * ? 等在加引号的情况下通常是可以创建目录的
    folder_name=$(echo "$regex_input" | sed 's/\//_/g')
    
    # 防止文件夹名为空
    if [ -z "$folder_name" ]; then
        folder_name="regex_task_$i"
    fi
    
    echo "创建输出目录: $folder_name"
    # 关键：必须使用引号 "$folder_name" 来处理包含特殊字符的名称
    mkdir -p "./$folder_name"
    
    # 运行 C++ 程序
    # 必须使用引号传递路径参数
    echo "$regex_input" | ./regex_automata "./$folder_name"
    
    # 3. 生成图片到该文件夹
    # 注意：所有的路径变量都加上了引号
    echo "正在生成可视化图片..."
    if command -v dot &> /dev/null; then
        if [ -f "./$folder_name/nfa_graph.dot" ]; then
            dot -Tpng "./$folder_name/nfa_graph.dot" -o "./$folder_name/nfa.png"
            echo "  -> 生成: ./$folder_name/nfa.png"
        fi

        if [ -f "./$folder_name/dfa_graph.dot" ]; then
            dot -Tpng "./$folder_name/dfa_graph.dot" -o "./$folder_name/dfa.png"
            echo "  -> 生成: ./$folder_name/dfa.png"
        fi
    else
        echo "警告: 未找到 'dot' 命令，无法生成 PNG 图片。"
    fi
    
    echo "完成: $regex_input"
done

echo "=== 全部任务完成 ==="