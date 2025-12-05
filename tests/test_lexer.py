#!/usr/bin/env python3
"""
自动化测试词法分析器 ./regex_automata 1
测试用例位于 ./lexer_cases/ 目录下的 .txt 文件中
"""
import subprocess
import sys
import re
import os
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
LEXER_EXE = PROJECT_ROOT / "regex_automata"

if not os.path.isfile(LEXER_EXE):
    print(f"❌ 找不到词法分析器: {os.path.abspath(LEXER_EXE)}")
    print("请确保在 CS2205-Lexer/ 目录下有可执行文件 'regex_automata'")
    sys.exit(1)


def parse_expected_tokens(lines):
    """从预期行中提取 (token_type, lexeme) 列表"""
    tokens = []
    for line in lines:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        match = re.match(r'^([A-Z_][A-Z0-9_]*)\s+"(.*)"$', line)
        if match:
            tokens.append((match.group(1), match.group(2)))
        else:
            print(f"⚠️ 警告：无法解析预期行: {line}")
    return tokens


def run_lexer_on_input(input_str):
    """运行 ./regex_automata 1 并返回实际 token 列表"""
    try:
        proc = subprocess.Popen(
            [LEXER_EXE, "1"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            cwd=PROJECT_ROOT,  # 确保在项目根目录运行
        )
        stdout, stderr = proc.communicate(input=input_str + "\nquit\n", timeout=10)

        if proc.returncode != 0:
            print(f"❌ 词法分析器崩溃，stderr:\n{stderr}")
            return None

        lines = stdout.splitlines()
        in_table = False
        actual_tokens = []
        for line in lines:
            if "Tokens:" in line:
                in_table = True
                continue
            if in_table and "Total:" in line:
                break
            if (
                in_table
                and "│" in line
                and "Token Type" not in line
                and "Line" not in line
            ):
                parts = [p.strip() for p in line.split("│")[1:-1]]
                if len(parts) >= 4:
                    token_type = parts[2]
                    lexeme = parts[3]
                    if lexeme.startswith('"') and lexeme.endswith('"'):
                        lexeme = lexeme[1:-1]
                    actual_tokens.append((token_type, lexeme))
        return actual_tokens

    except subprocess.TimeoutExpired:
        proc.kill()
        print("❌ 词法分析器超时（可能死循环）")
        return None
    except Exception as e:
        print(f"❌ 运行词法分析器出错: {e}")
        return None


def load_test_cases_from_file(file_path):
    """从单个文件加载测试用例"""
    test_cases = []
    current_input = None
    current_expected = []

    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if line.startswith(">>> "):
                if current_input is not None:
                    expected = parse_expected_tokens(current_expected)
                    test_cases.append((current_input, expected, str(file_path)))
                current_input = line[4:]
                current_expected = []
            else:
                current_expected.append(line)

        if current_input is not None:
            expected = parse_expected_tokens(current_expected)
            test_cases.append((current_input, expected, str(file_path)))

    return test_cases


def main():
    lexer_cases_dir = SCRIPT_DIR / "lexer_cases"
    if not lexer_cases_dir.exists():
        print(f"❌ 测试用例目录不存在: {lexer_cases_dir}")
        sys.exit(1)

    # 收集所有 .txt 测试文件
    test_files = list(lexer_cases_dir.glob("*.txt"))
    if not test_files:
        print(f"❌ 未找到任何测试文件 (*.txt) 在 {lexer_cases_dir}")
        sys.exit(1)

    all_test_cases = []
    for test_file in sorted(test_files):
        cases = load_test_cases_from_file(test_file)
        all_test_cases.extend(cases)

    print(f"🧪 共加载 {len(all_test_cases)} 个测试用例")
    print("-" * 50)

    passed = 0
    failed = 0

    for i, (input_str, expected, source_file) in enumerate(all_test_cases, 1):
        print(
            f"\n[{i}/{len(all_test_cases)}] 测试: {repr(input_str)} (来自 {os.path.basename(source_file)})"
        )
        actual = run_lexer_on_input(input_str)

        if actual is None:
            print("❌ 测试失败：词法分析器未正常返回")
            failed += 1
            continue

        if actual == expected:
            print("✅ 通过")
            passed += 1
        else:
            print("❌ 失败")
            print(f"  期望: {expected}")
            print(f"  实际: {actual}")
            failed += 1

    print("\n" + "=" * 60)
    print(f"✅ 总结: {passed} 通过, {failed} 失败")
    print("=" * 60)

    if failed > 0:
        sys.exit(1)
    else:
        print("🎉 所有测试通过！")


if __name__ == "__main__":
    main()
