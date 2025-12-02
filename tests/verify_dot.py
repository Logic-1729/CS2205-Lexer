# the version that uses random strings to test
# import sys
# import os
# import re
# import random
# import pydot

# alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"


# def random_string():
#     length = random.randint(0, 6)
#     return "".join(random.choice(alphabet) for _ in range(length))


# def expand_single_label(token: str):
#     """展开单个子标签为字符集合"""
#     chars = set()
#     if token.startswith("[") and token.endswith("]"):
#         # 处理字符集
#         content = token[1:-1]
#         i = 0
#         while i < len(content):
#             if i + 2 < len(content) and content[i + 1] == "-":
#                 # 范围，如 a-z
#                 start, end = content[i], content[i + 2]
#                 chars.update(chr(c) for c in range(ord(start), ord(end) + 1))
#                 i += 3
#             else:
#                 chars.add(content[i])
#                 i += 1
#     else:
#         # 普通字符串，逐字符展开
#         chars.update(token)
#     return chars


# def expand_label(label: str):
#     """
#     支持逗号分隔的多个子标签，例如 "[0-9],[a-y],z"
#     """
#     chars = set()
#     # 去掉可能的引号
#     label = label.strip('"')
#     # 按逗号分割
#     parts = [part.strip() for part in label.split(",")]
#     for part in parts:
#         if part:
#             chars.update(expand_single_label(part))
#     return chars


# def parse_dfa(dot_file):
#     graphs = pydot.graph_from_dot_file(dot_file)
#     graph = graphs[0]
#     edges = graph.get_edges()
#     nodes = graph.get_nodes()

#     # 起始状态
#     start_edges = [e for e in edges if e.get_source() == "__start0"]
#     start_state = start_edges[0].get_destination() if start_edges else None

#     # 接受状态
#     accepting = [
#         n.get_name() for n in nodes if (n.get_shape() or "").lower() == "doublecircle"
#     ]

#     # 转移表
#     transitions = {}
#     for e in edges:
#         src = e.get_source()
#         dst = e.get_destination()
#         label = e.get_label()
#         if src == "__start0":
#             continue
#         if label is None:
#             continue
#         label = str(label).strip('"')
#         for ch in expand_label(label):
#             transitions.setdefault(src, {})[ch] = dst

#     return start_state, accepting, transitions


# def simulate_dfa(start, accepting, transitions, s):
#     state = start
#     for ch in s:
#         if state not in transitions or ch not in transitions[state]:
#             return False
#         state = transitions[state][ch]
#     return state in accepting


# def verify_regex(regex):
#     dot_file = f"{regex}/dfa_graph.dot"
#     if not os.path.exists(dot_file):
#         return False, f"{regex}: dot文件不存在"

#     start, accepting, transitions = parse_dfa(dot_file)

#     for _ in range(50):
#         s = random_string()
#         expected = re.fullmatch(regex, s) is not None
#         actual = simulate_dfa(start, accepting, transitions, s)
#         if expected != actual:
#             return (
#                 False,
#                 f"{regex}: ❌ 错误样例 -> '{s}' (期望 {expected}, 实际 {actual})",
#             )

#     return True, f"{regex}: ✅ 验证通过"


# if __name__ == "__main__":
#     regex = sys.argv[1]
#     ok, msg = verify_regex(regex)
#     print(msg)


# the version that uses strings which are related to the regex to test, making the outcome more convincing
import sys
import os
import re
import random
import pydot

ALPHABET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

# 每个正则表达式测试的样例总数
EXAMPLE_COUNT = 50

# 随机字符串的长度范围
RANDOM_MIN_LEN = 0
RANDOM_MAX_LEN = 8

# dot 文件相关常量
DFA_DOT_FILENAME = "dfa_graph.dot"
START_EDGE_SOURCE = "__start0"
ACCEPT_NODE_SHAPE = "doublecircle"

# dot文件边标签分隔符
LABEL_SEPARATOR = ","


# 生成随机字符串
def random_string(min_len=RANDOM_MIN_LEN, max_len=RANDOM_MAX_LEN):
    length = random.randint(min_len, max_len)
    return "".join(random.choice(ALPHABET) for _ in range(length))


# ----- DOT label expansion -----


def expand_single_label(token: str):
    """
    解析单个标签片段（如 "[a-z]" 或 "x"），返回对应的字符集合。
    支持：
      - 字符范围：[a-z], [A-Z], [0-9]
      - 字面量：单个字符或字符串（如 "ab" 视为 {'a', 'b'}）
    """
    chars = set()
    token = token.strip()
    if token.startswith("[") and token.endswith("]"):
        # 处理范围形式，比如 [a-z]
        content = token[1:-1]
        i = 0
        while i < len(content):
            if i + 2 < len(content) and content[i + 1] == "-":
                start, end = content[i], content[i + 2]
                chars.update(chr(c) for c in range(ord(start), ord(end) + 1))
                i += 3
            else:
                chars.add(content[i])
                i += 1
    else:
        # 处理非范围形式，逐个字符添加
        for c in token:
            chars.add(c)
    return chars


def expand_label(label: str):
    """
    解析完整的边标签（可能包含逗号分隔的多部分），例如 '[0-9],[a-y],z',
    返回所有可能字符的并集。
    """
    chars = set()
    label = str(label).strip('"')  # 去除 Graphviz 自动加的双引号
    parts = [
        p.strip() for p in label.split(LABEL_SEPARATOR) if p.strip()
    ]  # 将逗号分隔的各部分写入列表，下一步用 expand_single_label 处理
    for part in parts:
        chars.update(expand_single_label(part))
    return chars


# ----- DFA parsing and simulation -----


def parse_dfa(dot_file):
    """
    从 DOT 文件中解析 DFA 结构：
      - 起始状态（通过 __start0 边确定）
      - 接受状态集合（形状为 doublecircle 的节点）
      - 转移函数：{ 当前状态: { 字符: 下一状态 } }
    """
    graphs = pydot.graph_from_dot_file(dot_file)
    graph = graphs[0]
    edges = graph.get_edges()
    nodes = graph.get_nodes()

    # 寻找起始状态(文件中 _start0 指向的节点)
    start_edges = [e for e in edges if e.get_source() == START_EDGE_SOURCE]
    start_state = start_edges[0].get_destination() if start_edges else None

    # 寻找所有接受状态
    accepting = [
        n.get_name()
        for n in nodes
        if (n.get_shape() or "").lower() == ACCEPT_NODE_SHAPE
    ]

    # 构建转移表
    transitions = {}
    for e in edges:
        src = e.get_source()
        dst = e.get_destination()
        label = e.get_label()
        # 忽略起始边和无标签边
        if src == START_EDGE_SOURCE or label is None:
            continue
        # 将标签展开为字符集合，并建立转移
        for ch in expand_label(label):
            transitions.setdefault(src, {})[ch] = dst
    return start_state, accepting, transitions


def simulate_dfa(start, accepting, transitions, s):
    """
    模拟 DFA 对字符串 s 的处理过程。
    返回 True 表示接受，False 表示拒绝。
    """
    if start is None:
        return False
    state = start
    for ch in s:
        if state not in transitions or ch not in transitions[state]:
            return False  # 没有转移路径，False
        state = transitions[state][ch]
    return state in accepting  # 最终状态是接受状态，True


# ----- Near-regex example generation -----


def mutate_string(s):
    """
    对字符串进行轻微扰动（用于生成反例）：
      - 替换一个字符
      - 插入一个字符
      - 删除一个字符
    """
    if not s:
        return random.choice(ALPHABET)
    ops = ["replace", "insert", "delete"]
    op = random.choice(ops)
    i = random.randrange(len(s))
    if op == "replace":
        c = random.choice(ALPHABET)
        return s[:i] + c + s[i + 1 :]
    elif op == "insert":
        c = random.choice(ALPHABET)
        return s[:i] + c + s[i:]
    else:  # delete
        return s[:i] + s[i + 1 :]


def simplify_alternations(regex):
    """
    简化正则表达式中的顶层选择结构（...|...），随机保留一个分支。
    例如：(a|b)c → ac 或 bc
    注意：不处理嵌套选择，仅用于生成“近似”正例。
    """
    i, n = 0, len(regex)
    out = []
    while i < n:
        if regex[i] == "(":
            depth = 1
            j = i + 1
            # 寻找匹配的右括号
            while j < n and depth > 0:
                if regex[j] == "(":
                    depth += 1
                elif regex[j] == ")":
                    depth -= 1
                j += 1
            group = regex[i + 1 : j - 1] if j <= n else regex[i + 1 :]
            # 按顶层 '|' 分割（忽略括号内的 |）
            parts = []
            buf, d = [], 0
            for ch in group:
                if ch == "(":
                    d += 1
                elif ch == ")":
                    d -= 1
                if ch == "|" and d == 0:
                    parts.append("".join(buf))
                    buf = []
                else:
                    buf.append(ch)
            parts.append("".join(buf))
            choice = random.choice(parts)  # 随机选择一个分支
            out.append(choice)
            i = j
        else:
            # 对于一般字符，直接添加
            out.append(regex[i])
            i += 1
    return "".join(out)


def tokenize_linear(regex):
    """
    将简化的正则表达式（无选择结构）切分为 (原子, 量词) 序列。
    原子：字符类（如 [a-z]）或单个字符
    量词：'', '?', '*', '+'
    """
    tokens = []
    i, n = 0, len(regex)
    while i < n:
        if regex[i] == "[":
            j = i + 1
            while j < n and regex[j] != "]":
                j += 1
            atom = regex[i : j + 1] if j < n else regex[i:n]
            i = j + 1
        else:
            atom = regex[i]
            i += 1
        quant = ""
        if i < n and regex[i] in "?*+":
            quant = regex[i]
            i += 1
        tokens.append((atom, quant))  # 如果一个原子后面没有量词，量词设置为空字符串
    return tokens


def gen_segment(atom, quant):
    """
    根据 (原子, 量词) 生成一段字符串。
    例如：('[a-z]', '*') → 随机生成 0~3 个小写字母
    """
    # 从原子中选一个代表字符(使用expand_single_label)
    choices = list(expand_single_label(atom)) if atom.startswith("[") else [atom]
    rep = random.choice(choices) if choices else ""
    if quant == "?":
        return rep if random.random() < 0.5 else ""
    elif quant == "*":
        k = random.randint(0, 3)
        return rep * k
    elif quant == "+":
        k = random.randint(1, 3)
        return rep * k
    else:
        return rep


def generate_near_examples(regex, count=EXAMPLE_COUNT):
    """
    生成贴近正则表达式的测试样例：
      1. 简化 regex（处理选择结构）
      2. 生成正例（根据量词规则）
      3. 通过扰动生成反例
      4. 所有样例均用 re.fullmatch 标注正确答案（避免假设错误）
      5. 若数量不足，用完全随机字符串补充
    """
    examples = []
    linear = simplify_alternations(regex)
    tokens = tokenize_linear(linear)

    # 生成正例
    positives = []
    for _ in range(max(10, count // 2)):
        s = "".join(gen_segment(atom, quant) for atom, quant in tokens)
        positives.append(s)

    # 扰动生成反例
    negatives = [mutate_string(s) for s in positives]

    # 用 re.fullmatch 标注真实结果
    labeled = []
    for s in positives + negatives:
        labeled.append((s, re.fullmatch(regex, s) is not None))

    # y用随机字符串补充至要求的数量
    while len(labeled) < count:
        s = random_string(RANDOM_MIN_LEN, RANDOM_MAX_LEN)
        labeled.append((s, re.fullmatch(regex, s) is not None))

    random.shuffle(labeled)
    return labeled[:count]


# ----- Verification -----


# Build full dot path from regex
def build_dot_path(regex: str) -> str:
    """根据正则表达式目录名，构建 DFA DOT 文件的完整路径"""
    return os.path.join(regex, DFA_DOT_FILENAME)


def verify_regex(regex):
    """
    验证指定正则表达式对应的 DFA 是否正确：
      - 检查 DOT 文件是否存在
      - 解析 DFA
      - 生成测试样例
      - 比对 DFA 与 re.fullmatch 的结果
      - 返回验证结果和错误信息
    """
    dot_file = build_dot_path(regex)
    if not os.path.exists(dot_file):
        return False, f"{regex}: dot文件不存在"

    start, accepting, transitions = parse_dfa(dot_file)

    examples = generate_near_examples(regex, EXAMPLE_COUNT)
    errors = []
    for s, expected in examples:
        actual = simulate_dfa(start, accepting, transitions, s)
        if expected != actual:
            errors.append(f"'{s}' (期望 {expected}, 实际 {actual})")

    if errors:
        return False, f"{regex}: ❌ 错误样例汇总:\n  " + "\n  ".join(errors)
    return True, f"{regex}: ✅ 验证通过"


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("用法: python3 verify_dot.py <regex>")
        sys.exit(2)
    regex = sys.argv[1]
    ok, msg = verify_regex(regex)
    print(msg)
