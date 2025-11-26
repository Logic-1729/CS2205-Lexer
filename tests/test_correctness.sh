#!/bin/bash
set -e

TEST_FILE="testcases/test_cases2.txt"

if [ ! -f "$TEST_FILE" ]; then
    echo "❌ 没有找到 $TEST_FILE"
    exit 1
fi

echo "=== 开始批量测试正则表达式 ==="

pass_count=0
fail_count=0

while IFS= read -r regex; do
    [ -z "$regex" ] && continue

    echo "------------------------------------------------"
    echo "正在测试正则: $regex"

    # 调用 regex_automata 生成 dot 文件
    echo "$regex" | ../regex_automata "$regex"

    # 验证 DFA 正确性
    result=$(python3 verify_dot.py "$regex")

    if [[ "$result" == *"验证通过"* ]]; then
        echo "$result"
        pass_count=$((pass_count+1))
    else
        # 错误时打印详细信息（包含错误样例）
        echo "$result"
        fail_count=$((fail_count+1))
    fi

    # 自动删除生成的文件夹
    if [ -d "$regex" ]; then
        rm -rf "$regex"
        echo "🗑 已删除测试目录: $regex"
    fi

done < "$TEST_FILE"

echo "=== 测试完成: 通过 $pass_count 条, 失败 $fail_count 条 ==="
