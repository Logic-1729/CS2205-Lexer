# # 命令行中输入 python3 gen_testcases.py 即可生成 total 个测试样例至 tests 文件夹下的 test_cases 文件里

# import os
# import random
# import re
# import sys

# # 常见正则模式
# regex_patterns = [
#     r"ab",  # 简单字符串
#     r"[a-z]+",  # 字母串
#     r"[0-9]+",  # 数字串
#     r"a*b",  # 零个或多个a后跟b
#     r"(ab|cd)",  # 并集
#     r"[A-Z][a-z]*",  # 首字母大写的单词
# ]

# # 针对每个正则，手动提供一些匹配样例
# match_examples = {
#     r"ab": ["ab"],
#     r"[a-z]+": ["hello", "xyz"],
#     r"[0-9]+": ["123", "2025"],
#     r"a*b": ["b", "ab", "aaab"],
#     r"(ab|cd)": ["ab", "cd"],
#     r"[A-Z][a-z]*": ["Hello", "World"],
# }

# alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"


# def random_string():
#     length = random.randint(1, 6)
#     return "".join(random.choice(alphabet) for _ in range(length))


# def generate_testcases(total=20):
#     cases = []
#     half = total // 2

#     # 生成 MATCH 样例
#     while len(cases) < half:
#         regex = random.choice(regex_patterns)
#         test_str = random.choice(match_examples[regex])
#         cases.append(f"{regex}\t{test_str}\tMATCH")

#     # 生成 NO_MATCH 样例
#     while len(cases) < total:
#         regex = random.choice(regex_patterns)
#         test_str = random_string()
#         try:
#             if not re.fullmatch(regex, test_str):
#                 cases.append(f"{regex}\t{test_str}\tNO_MATCH")
#         except re.error:
#             continue

#     return cases


# if __name__ == "__main__":
#     os.makedirs("tests", exist_ok=True)
#     total = 40
#     if len(sys.argv) > 1:
#         try:
#             total = int(sys.argv[1])
#         except ValueError:
#             pass
#     cases = generate_testcases(total)
#     with open("tests/test_cases9.txt", "w") as f:
#         for case in cases:
#             f.write(case + "\n")
#     print(f"✅ 已生成 {total} 条测试用例到 tests/test_cases1.txt")


# Generate true regex without MATCH and NO_MATCH
import os
import random
import sys

# ====== Configuration constants (edit here, avoid hardcoding below) ======

# 输出文件夹和文件名
OUTPUT_DIR = "testcases"
OUTPUT_FILENAME = "test_cases.txt"

# 默认样例数量(也就是生成正则表达式的数量)
DEFAULT_TOTAL = 40

# 生成正则表达式的长度限制
REGEX_LEN_MIN = 6
REGEX_LEN_MAX = 10

# 将串联结果用 top-leval alternation（... | ...）包裹的概率
UNION_PROB = 0.30

# 基础片段：字符集合、单字符、可选、循环、并集
FRAGMENTS = [
    r"[a-z0-9]",
    r"[A-Z]",
    r"a",
    r"b",
    r"c",
    r"d",
    r"x",
    r"y",
    r"z",
]


QUANTIFIERS = ["plain", "optional", "star", "plus"]

# ========================================================================


def wrap_fragment(frag: str) -> str:
    """Wrap a fragment with a random quantifier."""
    choice = random.choice(QUANTIFIERS)
    if choice == "plain":
        return frag
    if choice == "optional":
        return frag + "?"
    if choice == "star":
        return frag + "*"
    if choice == "plus":
        return frag + "+"
    return frag


def generate_regex() -> str:
    """Generate a longer regex by concatenating wrapped fragments, optionally adding a union."""
    length = random.randint(REGEX_LEN_MIN, REGEX_LEN_MAX)
    parts = [wrap_fragment(random.choice(FRAGMENTS)) for _ in range(length)]

    if random.random() < UNION_PROB and length >= 2:
        mid = len(parts) // 2
        left = "".join(parts[:mid])
        right = "".join(parts[mid:])
        return f"({left}|{right})"
    return "".join(parts)


def generate_testcases(total: int) -> list[str]:
    """Generate a list of regex test cases."""
    return [generate_regex() for _ in range(total)]


def build_output_path() -> str:
    """Compose the full output path from constants."""
    return os.path.join(OUTPUT_DIR, OUTPUT_FILENAME)


if __name__ == "__main__":
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    total = DEFAULT_TOTAL
    if len(sys.argv) > 1:
        try:
            total = int(sys.argv[1])
        except ValueError:
            pass

    cases = generate_testcases(total)
    out_path = build_output_path()

    with open(out_path, "w") as f:
        for case in cases:
            f.write(case + "\n")

    print(f"✅ 已生成 {total} 条正则表达式到 {out_path}")
