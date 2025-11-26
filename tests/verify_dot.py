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

# number of total examples to test per regex
EXAMPLE_COUNT = 50

# Fallback random string length range for filler samples
RANDOM_MIN_LEN = 0
RANDOM_MAX_LEN = 8

# DOT parsing specifics
DFA_DOT_FILENAME = "dfa_graph.dot"
START_EDGE_SOURCE = "__start0"
ACCEPT_NODE_SHAPE = "doublecircle"

# Label parsing
LABEL_SEPARATOR = ","


def random_string(min_len=RANDOM_MIN_LEN, max_len=RANDOM_MAX_LEN):
    length = random.randint(min_len, max_len)
    return "".join(random.choice(ALPHABET) for _ in range(length))


# ----- DOT label expansion -----


def expand_single_label(token: str):
    """Expand a single sublabel to a set of characters: [a-z], [A-Z], [0-9], or literal."""
    chars = set()
    token = token.strip()
    if token.startswith("[") and token.endswith("]"):
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
        # Treat as literal; if multi-char like "ab", expand to {'a', 'b'}
        for c in token:
            chars.add(c)
    return chars


def expand_label(label: str):
    """Support comma-separated label parts, e.g. '[0-9],[a-y],z'."""
    chars = set()
    label = str(label).strip('"')
    parts = [p.strip() for p in label.split(LABEL_SEPARATOR) if p.strip()]
    for part in parts:
        chars.update(expand_single_label(part))
    return chars


# ----- DFA parsing and simulation -----


def parse_dfa(dot_file):
    graphs = pydot.graph_from_dot_file(dot_file)
    graph = graphs[0]
    edges = graph.get_edges()
    nodes = graph.get_nodes()

    # Start state via __start0 -> S
    start_edges = [e for e in edges if e.get_source() == START_EDGE_SOURCE]
    start_state = start_edges[0].get_destination() if start_edges else None

    # Accepting states
    accepting = [
        n.get_name()
        for n in nodes
        if (n.get_shape() or "").lower() == ACCEPT_NODE_SHAPE
    ]

    # Transitions: state -> {char -> next_state}
    transitions = {}
    for e in edges:
        src = e.get_source()
        dst = e.get_destination()
        label = e.get_label()
        if src == START_EDGE_SOURCE or label is None:
            continue
        for ch in expand_label(label):
            transitions.setdefault(src, {})[ch] = dst
    return start_state, accepting, transitions


def simulate_dfa(start, accepting, transitions, s):
    if start is None:
        return False
    state = start
    for ch in s:
        if state not in transitions or ch not in transitions[state]:
            return False
        state = transitions[state][ch]
    return state in accepting


# ----- Near-regex example generation -----


def mutate_string(s):
    """Slightly perturb a string for negative examples."""
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
    Simplify top-level ( ... | ... ) groups by randomly picking one branch.
    Does not support nested alternations deeply; good enough for near examples.
    """
    i, n = 0, len(regex)
    out = []
    while i < n:
        if regex[i] == "(":
            depth = 1
            j = i + 1
            while j < n and depth > 0:
                if regex[j] == "(":
                    depth += 1
                elif regex[j] == ")":
                    depth -= 1
                j += 1
            group = regex[i + 1 : j - 1] if j <= n else regex[i + 1 :]
            # Split by top-level '|' within the group
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
            choice = random.choice(parts)
            out.append(choice)
            i = j
        else:
            out.append(regex[i])
            i += 1
    return "".join(out)


def tokenize_linear(regex):
    """
    Tokenize a linearized regex into [(atom, quant)]:
    - atom: '[...]' or single literal
    - quant: '', '?', '*', '+'
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
        tokens.append((atom, quant))
    return tokens


def gen_segment(atom, quant):
    """Generate a segment for (atom, quant)."""
    # Pick a representative char for atom
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
    Generate near-regex examples:
    - Positives: build strings from simplified alternations and quantifiers.
    - Negatives: mutate positives.
    Always label expected via re.fullmatch to avoid false assumptions.
    """
    examples = []
    linear = simplify_alternations(regex)
    tokens = tokenize_linear(linear)

    # Create positives
    positives = []
    for _ in range(max(10, count // 2)):
        s = "".join(gen_segment(atom, quant) for atom, quant in tokens)
        positives.append(s)

    # Create negatives by mutation
    negatives = [mutate_string(s) for s in positives]

    # Label all examples by actual regex truth
    labeled = []
    for s in positives + negatives:
        labeled.append((s, re.fullmatch(regex, s) is not None))

    # If fewer than count, add random samples as filler (also labeled)
    while len(labeled) < count:
        s = random_string(RANDOM_MIN_LEN, RANDOM_MAX_LEN)
        labeled.append((s, re.fullmatch(regex, s) is not None))

    random.shuffle(labeled)
    return labeled[:count]


# ----- Verification -----


# Build full dot path from regex
def build_dot_path(regex: str) -> str:
    return os.path.join(regex, DFA_DOT_FILENAME)


def verify_regex(regex):
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
