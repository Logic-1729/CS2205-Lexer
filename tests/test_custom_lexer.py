#!/usr/bin/env python3
import os
import sys
import subprocess
import re
import glob
from pathlib import Path

try:
    import yaml
except ImportError:
    print("❌ Please install pyyaml: pip install pyyaml", file=sys.stderr)
    sys.exit(1)

SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent  # 因为 tests/ 在项目根目录下
CASES_DIR = SCRIPT_DIR / "custom_cases"
EXECUTABLE = PROJECT_ROOT / "regex_automata"


def run_test_from_file(case_file: Path) -> bool:
    with open(case_file, "r", encoding="utf-8") as f:
        case = yaml.safe_load(f)

    name = case.get("name", case_file.stem)
    token_classes = case.get("token_classes", [])
    inputs = case.get("inputs", [])

    print(f"Running test: {name} ({case_file.name})")

    # 构造输入流
    input_lines = []
    input_lines.append("2")  # Custom Lexer Mode
    input_lines.append(str(len(token_classes)))

    for tc in token_classes:
        input_lines.append(tc["name"])
        input_lines.append(tc["regex"])

    for inp in inputs:
        input_lines.append(inp["lexeme"])
    input_lines.append("quit")  # 必须以 quit 退出

    full_input = "\n".join(input_lines) + "\n"

    # 启动程序
    try:
        result = subprocess.run(
            [str(EXECUTABLE)],
            input=full_input,
            text=True,
            capture_output=True,
            cwd=PROJECT_ROOT,
            timeout=10,
        )
    except subprocess.TimeoutExpired:
        print("  ❌ TIMEOUT")
        return False

    if result.returncode != 0:
        print("  ❌ CRASHED")
        print("stderr:", result.stderr)
        return False

    output = result.stdout

    # 从输出中提取 token 行
    # 匹配: │    1 │      1 │ cd               │ "abcccc"               │
    matches = re.findall(r'│\s*\d+\s*│\s*\d+\s*│\s*(\S+)\s*│\s*"([^"]*)"\s*│', output)
    actual_tokens = [(token_type, lexeme) for token_type, lexeme in matches]

    if len(actual_tokens) != len(inputs):
        print(f"  ❌ Expected {len(inputs)} tokens, got {len(actual_tokens)}")
        print("Output:\n" + output)
        return False

    for i, (expected_type, expected_lexeme) in enumerate(
        [(inp["expected_token"], inp["lexeme"]) for inp in inputs]
    ):
        actual_type, actual_lexeme = actual_tokens[i]
        if actual_lexeme != expected_lexeme:
            print(
                f"  ❌ Lexeme mismatch on input {i+1}: expected '{expected_lexeme}', got '{actual_lexeme}'"
            )
            return False
        if actual_type != expected_type:
            print(
                f"  ❌ Token type mismatch for '{expected_lexeme}': expected '{expected_type}', got '{actual_type}'"
            )
            return False

    print("  ✅ PASSED")
    return True


def main():
    if not EXECUTABLE.exists():
        print(f"❌ Executable not found: {EXECUTABLE}", file=sys.stderr)
        sys.exit(1)

    if not CASES_DIR.is_dir():
        print(f"❌ Test cases directory not found: {CASES_DIR}", file=sys.stderr)
        sys.exit(1)

    yaml_files = sorted(glob.glob(str(CASES_DIR / "*.yaml")))
    if not yaml_files:
        print(f"⚠️  No .yaml test files found in {CASES_DIR}")
        return

    passed = 0
    total = len(yaml_files)

    for yaml_file in yaml_files:
        if run_test_from_file(Path(yaml_file)):
            passed += 1

    print(f"\n📊 Summary: {passed}/{total} tests passed.")
    sys.exit(0 if passed == total else 1)


if __name__ == "__main__":
    main()
