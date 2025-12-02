#!/bin/bash

# 只在命令明确失败时停止（但允许继续生成图片）
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
echo "  1. 预定义词法分析器 (lang. l tokens)"
echo "  2. 自定义词法分析器 (定义您自己的 tokens)"
echo "  3. 单个正则表达式转换 (Regex -> NFA -> DFA)"
echo ""
read -p "请输入模式 (1-3): " mode_choice

case $mode_choice in
    1)
        echo ""
        echo "=== 模式 1: 预定义词法分析器 ==="
        echo ""
        
        chmod +x ./regex_automata
        ./regex_automata 1
        
        if [ -f "lexer_dfa.dot" ]; then
            echo ""
            echo "=== 生成可视化图片 ==="
            if command -v dot &> /dev/null; then
                dot -Tpng lexer_dfa.dot -o lexer_dfa.png
                echo "✓ 生成: lexer_dfa.png"
            else
                echo "⚠ 警告: 未找到 'dot' 命令。"
            fi
        fi
        ;;
        
    2)
        echo ""
        echo "=== 模式 2: 自定义词法分析器 ==="
        echo ""
        
        read -p "请输入 token 类型数量: " token_count
        
        if !  [[ "$token_count" =~ ^[0-9]+$ ]]; then
            echo "❌ 错误: 请输入有效的数字。"
            exit 1
        fi
        
        echo "$token_count" > /tmp/lexer_input.txt
        
        for ((i=1; i<=token_count; i++))
        do
            echo ""
            echo "--- Token 类型 $i/$token_count ---"
            read -r -p "  名称 (如 IDENTIFIER): " token_name
            read -r -p "  正则表达式 (如 [a-z]+): " token_regex
            
            echo "$token_name" >> /tmp/lexer_input. txt
            echo "$token_regex" >> /tmp/lexer_input.txt
        done
        
        echo ""
        echo "--- 配置完成，进入交互模式 ---"
        
        chmod +x ./regex_automata
        (cat /tmp/lexer_input. txt; cat) | ./regex_automata 2
        
        rm -f /tmp/lexer_input. txt
        
        if [ -f "custom_lexer_dfa.dot" ]; then
            echo ""
            echo "=== 生成可视化图片 ==="
            if command -v dot &> /dev/null; then
                dot -Tpng custom_lexer_dfa. dot -o custom_lexer_dfa.png
                echo "✓ 生成: custom_lexer_dfa.png"
            fi
        fi
        ;;
        
    3)
        echo ""
        echo "=== 模式 3: 单个正则表达式转换 ==="
        echo ""
        
        read -p "请输入要处理的正则表达式个数: " count
        
        if ! [[ "$count" =~ ^[0-9]+$ ]]; then
            echo "❌ 错误: 请输入有效的数字。"
            exit 1
        fi
        
        for ((i=1; i<=count; i++))
        do
            echo ""
            echo "================================================"
            echo "  正在处理第 $i 个正则表达式 (共 $count 个)"
            echo "================================================"
            
            read -r -p "请输入正则表达式: " regex_input
            
            folder_name=$(echo "$regex_input" | sed 's/[\/:]/_/g' | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
            
            if [ ${#folder_name} -gt 100 ]; then
                folder_name="${folder_name:0:97}..."
            fi
            
            if [ -z "$folder_name" ]; then
                folder_name="regex_task_$i"
            fi
            
            echo ""
            echo "→ 输出目录: $folder_name"
            echo "  原始表达式: $regex_input"
            
            # 如果目录已存在，先删除旧文件
            if [ -d "$folder_name" ]; then
                echo "  清理旧文件..."
                rm -f "$folder_name"/*.png "$folder_name"/*.dot 2>/dev/null || true
            fi
            
            # 创建输出目录
            mkdir -p "$folder_name"
            
            # 运行程序
            chmod +x ./regex_automata
            echo "$regex_input" | ./regex_automata 3 "$folder_name"
            
            # 生成图片 - 关键：使用 || true 防止失败时退出
            echo ""
            echo "=== 生成可视化图片 ==="
            
            if command -v dot &> /dev/null; then
                png_generated=0
                
                # NFA
                nfa_dot="$folder_name/nfa_graph.dot"
                if [ -f "$nfa_dot" ]; then
                    echo "  正在生成 NFA 图片..."
                    # 使用 || true 确保即使失败也继续
                    dot -Tpng "$nfa_dot" -o "$folder_name/nfa. png" && {
                        echo "  ✓ NFA:     $folder_name/nfa.png"
                        ((png_generated++))
                    } || {
                        echo "  ✗ NFA 生成失败"
                    }
                else
                    echo "  ✗ 未找到: $nfa_dot"
                fi

                # DFA
                dfa_dot="$folder_name/dfa_graph.dot"
                if [ -f "$dfa_dot" ]; then
                    echo "  正在生成 DFA 图片..."
                    dot -Tpng "$dfa_dot" -o "$folder_name/dfa.png" && {
                        echo "  ✓ DFA:     $folder_name/dfa.png"
                        ((png_generated++))
                    } || {
                        echo "  ✗ DFA 生成失败"
                    }
                else
                    echo "  ✗ 未找到: $dfa_dot"
                fi

                # Min-DFA
                min_dfa_dot="$folder_name/min_dfa_graph.dot"
                if [ -f "$min_dfa_dot" ]; then
                    echo "  正在生成最小化 DFA 图片..."
                    dot -Tpng "$min_dfa_dot" -o "$folder_name/min_dfa.png" && {
                        echo "  ✓ Min-DFA: $folder_name/min_dfa.png"
                        ((png_generated++))
                    } || {
                        echo "  ✗ Min-DFA 生成失败"
                    }
                else
                    echo "  ✗ 未找到: $min_dfa_dot"
                fi
                
                echo ""
                if [ $png_generated -eq 0 ]; then
                    echo "  ⚠ 未生成任何图片"
                elif [ $png_generated -eq 3 ]; then
                    echo "  ✓ 成功生成全部 3 个图片"
                else
                    echo "  ⚠ 部分成功: 生成 $png_generated / 3 个图片"
                fi
            else
                echo "  ⚠ 警告: 未找到 'dot' 命令"
                echo "  安装方法:"
                echo "    Ubuntu/Debian: sudo apt-get install graphviz"
                echo "    macOS:         brew install graphviz"
            fi
            
            echo ""
            echo "✓ 完成: $regex_input"
            echo "  文件位置: $folder_name/"
            
            # 列出生成的文件
            if [ -d "$folder_name" ]; then
                echo "  生成的文件:"
                ls -lh "$folder_name/" 2>/dev/null | grep -v "^total" | awk '{printf "    - %-30s %6s\n", $9, $5}' || echo "    (无文件)"
            fi
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
echo "  - 查看生成的 . dot 文件以了解自动机结构"
echo "  - 使用图片查看器打开 .png 图片"
echo "  - 重新运行: ./build_and_run.sh"
echo ""