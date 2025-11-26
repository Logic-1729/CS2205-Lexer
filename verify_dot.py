import sys
import os
import re
import random
import pydot

alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"


def random_string():
    length = random.randint(0, 6)
    return "".join(random.choice(alphabet) for _ in range(length))


def expand_single_label(token: str):
    """展开单个子标签为字符集合"""
    chars = set()
    if token.startswith("[") and token.endswith("]"):
        # 处理字符集
        content = token[1:-1]
        i = 0
        while i < len(content):
            if i + 2 < len(content) and content[i + 1] == "-":
                # 范围，如 a-z
                start, end = content[i], content[i + 2]
                chars.update(chr(c) for c in range(ord(start), ord(end) + 1))
                i += 3
            else:
                chars.add(content[i])
                i += 1
    else:
        # 普通字符串，逐字符展开
        chars.update(token)
    return chars


def expand_label(label: str):
    """
    支持逗号分隔的多个子标签，例如 "[0-9],[a-y],z"
    """
    chars = set()
    # 去掉可能的引号
    label = label.strip('"')
    # 按逗号分割
    parts = [part.strip() for part in label.split(",")]
    for part in parts:
        if part:
            chars.update(expand_single_label(part))
    return chars


def parse_dfa(dot_file):
    graphs = pydot.graph_from_dot_file(dot_file)
    graph = graphs[0]
    edges = graph.get_edges()
    nodes = graph.get_nodes()

    # 起始状态
    start_edges = [e for e in edges if e.get_source() == "__start0"]
    start_state = start_edges[0].get_destination() if start_edges else None

    # 接受状态
    accepting = [
        n.get_name() for n in nodes if (n.get_shape() or "").lower() == "doublecircle"
    ]

    # 转移表
    transitions = {}
    for e in edges:
        src = e.get_source()
        dst = e.get_destination()
        label = e.get_label()
        if src == "__start0":
            continue
        if label is None:
            continue
        label = str(label).strip('"')
        for ch in expand_label(label):
            transitions.setdefault(src, {})[ch] = dst

    return start_state, accepting, transitions


def simulate_dfa(start, accepting, transitions, s):
    state = start
    for ch in s:
        if state not in transitions or ch not in transitions[state]:
            return False
        state = transitions[state][ch]
    return state in accepting


def verify_regex(regex):
    dot_file = f"{regex}/dfa_graph.dot"
    if not os.path.exists(dot_file):
        return False, f"{regex}: dot文件不存在"

    start, accepting, transitions = parse_dfa(dot_file)

    for _ in range(50):
        s = random_string()
        expected = re.fullmatch(regex, s) is not None
        actual = simulate_dfa(start, accepting, transitions, s)
        if expected != actual:
            return (
                False,
                f"{regex}: ❌ 错误样例 -> '{s}' (期望 {expected}, 实际 {actual})",
            )

    return True, f"{regex}: ✅ 验证通过"


if __name__ == "__main__":
    regex = sys.argv[1]
    ok, msg = verify_regex(regex)
    print(msg)
