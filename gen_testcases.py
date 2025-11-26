# # 命令行中输入 python3 gen_testcases.py 即可生成 total 个测试样例至 tests 文件夹下的 test_cases 文件里

# import os
# import random
# import re
# import sys

# # 常见正则模式（不包含字符串字面量）
# regex_patterns = [
#     r"ab",  # 简单字符串
#     r"[a-z]+",  # 字母串
#     r"[0-9]+",  # 数字串
#     r"a*b",  # 零个或多个a后跟b
#     r"(ab|cd)",  # 并集
#     r"[A-Z][a-z]*",  # 首字母大写的单词
# ]

# # 针对每个正则，手工提供一些匹配样例
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

#     # 先生成 MATCH 样例
#     while len(cases) < half:
#         regex = random.choice(regex_patterns)
#         test_str = random.choice(match_examples[regex])
#         cases.append(f"{regex}\t{test_str}\tMATCH")

#     # 再生成 NO_MATCH 样例
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

# 命令行中输入 python3 gen_testcases.py 即可生成 total 个测试样例至 tests 文件夹下的 test_cases 文件里

# 命令行中输入 python3 gen_testcases.py 即可生成 total 个正则表达式至 tests 文件夹下的 test_cases 文件里


# Generate true regex without MATCH and NO_MATCH
import os
import random
import sys

# 基础片段：字符集合、单字符、可选、循环、并集
fragments = [
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


# 包装器：生成更复杂的结构
def wrap_fragment(frag):
    choice = random.choice(["plain", "optional", "star", "plus"])
    if choice == "plain":
        return frag
    elif choice == "optional":
        return frag + "?"
    elif choice == "star":
        return frag + "*"
    elif choice == "plus":
        return frag + "+"
    return frag


# 生成一个较长的正则表达式
def generate_regex():
    length = random.randint(6, 10)  # 由多个片段组成
    parts = []
    for _ in range(length):
        frag = random.choice(fragments)
        parts.append(wrap_fragment(frag))
    # 随机插入并集
    if random.random() < 0.3:
        left = "".join(parts[: len(parts) // 2])
        right = "".join(parts[len(parts) // 2 :])
        return f"({left}|{right})"
    else:
        return "".join(parts)


def generate_testcases(total=20):
    cases = []
    for _ in range(total):
        regex = generate_regex()
        cases.append(regex)
    return cases


if __name__ == "__main__":
    os.makedirs("tests", exist_ok=True)
    total = 40
    if len(sys.argv) > 1:
        try:
            total = int(sys.argv[1])
        except ValueError:
            pass
    cases = generate_testcases(total)
    with open("tests/test_cases9.txt", "w") as f:
        for case in cases:
            f.write(case + "\n")
    print(f"✅ 已生成 {total} 条正则表达式到 tests/test_cases.txt")
