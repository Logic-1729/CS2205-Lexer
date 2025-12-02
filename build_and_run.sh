#!/bin/bash

# 确保脚本抛出错误时停止执行
set -e

echo "==========================================="
echo "  CS2205 Lexical Analyzer - Build Script"
echo "==========================================="
echo ""

# 创建构建目录
echo "=== 步骤 1: 配置构建环境 ==="
mkdir -p build
cd build

# 运行 CMake 配置
echo "运行 CMake 配置..."
cmake ..

# 编译项目
echo ""
echo "=== 步骤 2: 编译项目 ==="
cmake --build .

# 将生成的可执行文件复制到根目录
cp regex_automata ..
cd .. 

echo ""
echo "✓ 编译成功！"
echo "  可执行文件: ./regex_automata"
echo ""

# 询问运行模式
echo "==========================================="
echo "  选择运行模式"
echo "==========================================="
echo "  1. 预定义词法分析器 (lang.l tokens)"
echo "  2. 自定义词法分析器 (定义您自己的 tokens)"
echo "  3. 单个正则表达式转换 (Regex -> NFA -> DFA)"
echo ""
read -p "请输入模式 (1-3): " mode_choice

case $mode_choice in
    1)
        echo ""
        echo "=== 模式 1: 预定义词法分析器 ==="
        echo ""
        
        # 交互式运行
        chmod +x ./regex_automata
        # 【修正】直接运行，不使用管道，以支持交互
        ./regex_automata 1
        
        # 生成 DFA 可视化
        if [ -f "lexer_dfa.dot" ]; then
            echo ""
            echo "=== 生成可视化图片 ==="
            if command -v dot &> /dev/null; then
                dot -Tpng lexer_dfa.dot -o lexer_dfa.png
                echo "✓ 生成: lexer_dfa.png"
            else
                echo "⚠ 警告: 未找到 'dot' 命令，无法生成 PNG 图片。"
                echo "  请安装 Graphviz: sudo apt-get install graphviz"
            fi
        fi
        ;;
        
    2)
        echo ""
        echo "=== 模式 2: 自定义词法分析器 ==="
        echo ""
        
        # 询问 token 类型数量
        read -p "请输入 token 类型数量: " token_count
        
        # 校验输入
        if ! [[ "$token_count" =~ ^[0-9]+$ ]]; then
            echo "❌ 错误: 请输入有效的数字。"
            exit 1
        fi
        
        # 准备输入数据 (Token 数量)
        # 【修正】不再写入 "2" (模式选择已通过参数传递)
        echo "$token_count" > /tmp/lexer_input.txt
        
        # 收集每个 token 的定义
        for ((i=1; i<=token_count; i++))
        do
            echo ""
            echo "--- Token 类型 $i/$token_count ---"
            read -r -p "  名称 (如 IDENTIFIER): " token_name
            read -r -p "  正则表达式 (如 [a-z]+): " token_regex
            
            echo "$token_name" >> /tmp/lexer_input.txt
            echo "$token_regex" >> /tmp/lexer_input.txt
        done
        
        # 【修正】不写入 "quit"，以便后续转入交互模式
        
        echo ""
        echo "--- 配置完成，进入交互模式 ---"
        
        # 运行程序
        chmod +x ./regex_automata
        # 【修正】将配置文件和当前 stdin (cat) 拼接后传给程序
        # 这样程序先读配置，再读取用户键盘输入
        (cat /tmp/lexer_input.txt; cat) | ./regex_automata 2
        
        # 清理临时文件
        rm -f /tmp/lexer_input.txt
        
        # 生成可视化
        if [ -f "custom_lexer_dfa.dot" ]; then
            echo ""
            echo "=== 生成可视化图片 ==="
            if command -v dot &> /dev/null; then
                dot -Tpng custom_lexer_dfa.dot -o custom_lexer_dfa.png
                echo "✓ 生成: custom_lexer_dfa.png"
            else
                echo "⚠ 警告: 未找到 'dot' 命令。"
            fi
        fi
        ;;
        
    3)
        echo ""
        echo "=== 模式 3: 单个正则表达式转换 ==="
        echo ""
        
        # 询问要处理的正则表达式个数
        read -p "请输入要处理的正则表达式个数: " count
        
        # 校验输入
        if ! [[ "$count" =~ ^[0-9]+$ ]]; then
            echo "❌ 错误: 请输入有效的数字。"
            exit 1
        fi
        
        # 循环处理每个正则表达式
        for ((i=1; i<=count; i++))
        do
            echo ""
            echo "================================================"
            echo "  正在处理第 $i 个正则表达式 (共 $count 个)"
            echo "================================================"
            
            # 读取正则表达式 (保留反斜杠)
            read -r -p "请输入正则表达式: " regex_input
            
            # 设置文件夹名 (清理特殊字符但保留反斜杠)
            folder_name=$(echo "$regex_input" | sed 's/[\/\*\? ]/_/g' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
            
            # 限制文件夹名长度
            if [ ${#folder_name} -gt 50 ]; then
                folder_name="${folder_name:0:47}..."
            fi
            
            # 如果为空，使用默认名
            if [ -z "$folder_name" ]; then
                folder_name="regex_task_$i"
            fi
            
            echo ""
            echo "→ 输出目录: $folder_name"
            mkdir -p "./$folder_name"
            
            # 运行 C++ 程序
            chmod +x ./regex_automata
            # 【修正】通过参数传递模式 3 和输出目录
            echo "$regex_input" | ./regex_automata 3 "./$folder_name"
            
            # 生成图片
            echo ""
            echo "=== 生成可视化图片 ==="
            
            if command -v dot &> /dev/null; then
                png_generated=0
                
                # NFA
                if [ -f "./$folder_name/nfa_graph.dot" ]; then
                    dot -Tpng "./$folder_name/nfa_graph.dot" -o "./$folder_name/nfa.png"
                    echo "  ✓ NFA:  ./$folder_name/nfa.png"
                    ((png_generated++))
                fi

                # Original DFA
                if [ -f "./$folder_name/dfa_graph.dot" ]; then
                    dot -Tpng "./$folder_name/dfa_graph.dot" -o "./$folder_name/dfa.png"
                    echo "  ✓ DFA:  ./$folder_name/dfa.png"
                    ((png_generated++))
                fi

                # Minimized DFA
                if [ -f "./$folder_name/min_dfa_graph.dot" ]; then
                    dot -Tpng "./$folder_name/min_dfa_graph.dot" -o "./$folder_name/min_dfa.png"
                    echo "  ✓ Min-DFA: ./$folder_name/min_dfa.png"
                    ((png_generated++))
                fi
                
                if [ $png_generated -eq 0 ]; then
                    echo "  ⚠ 未找到 .dot 文件 (检查程序输出目录是否正确)"
                fi
            else
                echo "  ⚠ 警告: 未找到 'dot' 命令，无法生成 PNG 图片。"
                echo "  安装方法: sudo apt-get install graphviz"
            fi
            
            echo ""
            echo "✓ 完成: $regex_input"
        done
        
        echo ""
        echo "================================================"
        echo "  全部 $count 个正则表达式处理完成！"
        echo "================================================"
        ;;
        
    *)
        echo ""
        echo "❌ 错误: 无效的选择。请输入 1-3。"
        exit 1
        ;;
esac

echo ""
echo "==========================================="
echo "  任务完成"
echo "==========================================="
echo ""
echo "提示："
echo "  - 查看生成的 .dot 文件以了解自动机结构"
echo "  - 使用 Graphviz 查看器打开 .png 图片"
echo "  - 重新运行: ./build_and_run.sh"
echo ""