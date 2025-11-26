#!/bin/bash

# 确保脚本抛出错误时停止执行
set -e

echo "=== 正在构建 Regex Automata 项目 ==="

# 创建构建目录
mkdir -p build
cd build

# 运行 CMake 配置
cmake ..

# 编译项目
cmake --build .

# 将生成的可执行文件复制到根目录
cp regex_automata ..
cd ..

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
    # 使用 -r 防止反斜杠被转义消费
    read -r -p "请输入正则表达式: " regex_input
    
    # 2. 设置文件夹名
    # 移除双引号、单引号、反斜杠，并将斜杠转换为下划线
    folder_name=$(echo "$regex_input" | tr -d '"\\' | tr -d "'" | sed 's/\//_/g')
    
    if [ -z "$folder_name" ]; then
        folder_name="regex_task_$i"
    fi
    
    echo "创建输出目录: $folder_name"
    mkdir -p "./$folder_name"
    
    # 运行 C++ 程序
    chmod +x ./regex_automata
    # 注意：这里 echo 也要小心，不要让 shell 再次处理反斜杠
    echo "$regex_input" | ./regex_automata "./$folder_name"
    
    # 3. 生成图片
    echo "正在生成可视化图片..."
    if command -v dot &> /dev/null; then
        # NFA
        if [ -f "./$folder_name/nfa_graph.dot" ]; then
            dot -Tpng "./$folder_name/nfa_graph.dot" -o "./$folder_name/nfa.png"
            echo "  -> 生成: ./$folder_name/nfa.png"
        fi

        # Original DFA
        if [ -f "./$folder_name/dfa_graph.dot" ]; then
            dot -Tpng "./$folder_name/dfa_graph.dot" -o "./$folder_name/dfa.png"
            echo "  -> 生成: ./$folder_name/dfa.png"
        fi

        # Minimized DFA (新增)
        if [ -f "./$folder_name/min_dfa_graph.dot" ]; then
            dot -Tpng "./$folder_name/min_dfa_graph.dot" -o "./$folder_name/min_dfa.png"
            echo "  -> 生成: ./$folder_name/min_dfa.png"
        fi
    else
        echo "警告: 未找到 'dot' 命令，无法生成 PNG 图片。"
    fi
    
    echo "完成: $regex_input"
done

echo "=== 全部任务完成 ==="