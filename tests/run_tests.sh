#!/bin/bash
# 自动化测试 Regex Automata 项目

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXEC="$SCRIPT_DIR/../regex_automata"
TEST_FILE="$SCRIPT_DIR/test_cases.txt"

# 确保 test_cases.txt 存在
if [ ! -f "$TEST_FILE" ]; then
    echo "❌ 没有找到 $TEST_FILE，请先生成测试用例文件"
    exit 1
fi

pass_count=0
fail_count=0

# 遍历测试用例文件，使用 TAB 分隔
while IFS=$'\t' read -r regex input expected; do
    # 跳过空行
    [ -z "$regex" ] && continue

    echo "=== 测试正则: $regex 输入: '$input' ==="

    # 用 Python 的 re.fullmatch 来验证
    result=$(python3 - <<PYCODE
import re
regex = r"""$regex"""
test_str = """$input"""
expected = """$expected"""
try:
    if re.fullmatch(regex, test_str):
        result = "MATCH"
    else:
        result = "NO_MATCH"
    if result == expected:
        print("PASS")
    else:
        print("FAIL")
except Exception as e:
    print("ERROR")
PYCODE
)

    if [ "$result" = "PASS" ]; then
        echo "✅ 通过: 正则=$regex 输入=$input"
        pass_count=$((pass_count+1))
    elif [ "$result" = "FAIL" ]; then
        echo "❌ 失败: 正则=$regex 输入=$input"
        fail_count=$((fail_count+1))
    else
        echo "⚠️ 正则错误: $regex"
        fail_count=$((fail_count+1))
    fi

done < "$TEST_FILE"

echo "=== 测试完成: 通过 $pass_count 条, 失败 $fail_count 条 ==="
