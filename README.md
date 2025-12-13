# Regex to NFA to DFA Converter

这是一个基于 C++ 实现的正则表达式引擎核心组件演示项目。该项目能够解析正则表达式，将其转换为非确定性有限自动机 (NFA)，并进一步转换为确定性有限自动机 (DFA)，同时支持DFA最小化与完整词法分析器 (Lexer) 构建。

## 项目原理

本项目遵循经典的词法分析生成器设计原理，主要流程如下：

### 1. 正则表达式预处理 (Preprocessing)
*   **Token化**: 将输入字符串解析为 Token 流，识别操作符（`*`, `|`, `?`, `+`）和操作数。
*   **显式连接符**: 自动在相邻的操作数之间插入显式的连接符 `&`，简化后续解析逻辑。
*   **字符集处理**: 支持 `[...]` 语法和字符串字面量（如`"abc"`），将其解析为字符区间集合（`CharSet`，支持范围如`a-z`和转义字符如`\n`），而非简单的字符串。

### 2. 语法糖简化 (Regex Simplification)

*   将 `?`（零次或一次）重写为 `(X|ε)`，`+`（一次或多次）重写为 `XX*`，统一为仅含 `*`, `|`, `&`, `(`, `)` 的核心操作集。

### 3. 中缀转后缀 (Infix to Postfix)
*   使用 **Shunting-yard 算法** 将中缀正则表达式转换为后缀表达式（逆波兰表达式），处理操作符优先级（`*` > `+` > `&` > `|`）。

### 4. NFA 构建 (Thompson's Construction)
*   采用 **Thompson 构造法**，通过递归组合小的 NFA 片段构建复杂的 NFA。
*   **智能指针管理**: 使用 `std::shared_ptr` 管理节点内存，避免内存泄漏。
*   支持的操作：
    *   **Union (|)**: 并联。
    *   **Concatenation (&)**: 串联（含节点合并优化）。
    *   **Kleene Star (*)**: 闭包。
    *   **Option (?)**: 零次或一次（语法糖）。
    *   **Plus (+)**: 一次或多次（语法糖）。

### 5. DFA 转换 (Subset Construction) 
*   采用 **子集构造法 (Powerset Construction)**。
*   **Epsilon-Closure 缓存**: 优化了闭包计算，使用缓存避免重复遍历，提升性能。

### 6. DFA 最小化 (Minimization)
*   实现基于 **区分细化 (Partition Refinement)** 的最小化算法，合并等价状态，生成最简 DFA。

### 7. 词法分析器 (Lexer) 生成
*   支持 **多正则表达式联合编译**： 将多个 token 规则（如关键字、标识符、数字）合并为一个统一的 NFA，再转为单个 DFA。
*   **优先级规则**：按照“匹配字符串长度第一优先，匹配规则先后次序第二优先”的规则，解决歧义。
*   提供**预定义 token 集**（lang.l）和**自定义 token 模式**。

## 代码结构

项目源码位于 `src/` 目录下：

| 文件名                      | 功能描述                                           |
|:-------------------------|:-----------------------------------------------|
| `main.cpp`               | 程序入口。处理命令行参数，协调解析、构建及输出流程。现支持三种模式。             |
| `nfa.h`                  | 定义核心数据结构：`Node`, `Edge`, `NFAUnit`, `CharSet`。 |
| `dfa.h`                  | 定义 DFA 相关结构：`DFAState`, `DFATransition`。       |
| `lexer.h` / `lexer.cpp`  | 词法分析器类，支持多 token DFA 构建与 tokenization。         |
| `regex_parser.h`         | 定义解析器接口、Token 结构及异常类 `RegexSyntaxError`。       |
| `regex_simplifier.cpp`   | 将 `?` 和 `+` 语法糖转换为核心操作符。                       |
| `regex_preprocessor.cpp` | 实现正则预处理、字符集解析及 Token 流生成。                      |
| `infix_to_postfix.cpp`   | 实现 Shunting-yard 算法。                           |
| `nfa_builder.cpp`        | 实现 Thompson 构造法构建 NFA。                         |
| `dfa_converter.cpp`      | 子集构造算法实现 NFA 到 DFA 的转换逻辑（含闭包缓存）。               |
| `dfa_minimizer.cpp`      | 实现分区细化算法得到最小化 DFA。                             |
| `visualize.cpp`          | 负责生成 Graphviz `.dot` 文件。                       |

## 环境配置

### 1. 基础环境
*   **操作系统**: Linux / macOS (推荐), Windows (需 WSL 或 MinGW)。
*   **编译器**: 支持 **C++17** 的编译器（GCC 7+, Clang 5+, MSVC 19.14+）。
*   **构建系统**: **CMake** 3.10 或更高版本。

### 2. 可视化工具
*   **Graphviz**: 用于将生成的 `.dot` 文件转换为图片。
    *   Ubuntu/Debian: `sudo apt-get install graphviz`
    *   macOS: `brew install graphviz`

## 构建与运行

项目提供了一个自动化脚本 `build_and_run.sh`，集成了构建、运行和图片生成流程。

### 快速开始

1.  **赋予脚本权限**:
    ```bash
    chmod +x build_and_run.sh
    ```

2.  **运行脚本**:
    ```bash
    ./build_and_run.sh
    ```

3.  **交互式输入**:
    脚本会提示用户选择需要进行的模式，并根据选择结果为后续输入给出不同提示。 

### 手动构建 (CMake)

如果您只需编译而不运行测试脚本：

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

生成的可执行文件为 `regex_automata`。

### 运行参数

编译后，可直接调用可执行文件进入不同模式：

```bash
./regex_automata 1              # 预定义 lexer
./regex_automata 2              # 自定义 lexer
./regex_automata 3 "output_dir" # 正则表达式转换，输出到指定目录
```

下面是对三种运行模式的说明：

#### 模式 1：预定义 lexer
*   使用内置 token 规则 (参考lang.l) 。
*   交互式输入代码片段，实时输出 token 流。

#### 模式 2：自定义 lexer
*    交互式定义 token 类别 （名称 + 正则表达式）。
*    自动构建 Lexer DFA 并可视化。
*    可对任意输入进行 tokenization。
*    交互示例：
```bash
name: "Example from log"
token_classes:
  - name: ab
    regex: abccc
  - name: cd
    regex: abc*
  - name: ef
    regex: abcccc
inputs:
  - lexeme: "abccc"
    expected_token: "ab"
  - lexeme: "abcccc"
    expected_token: "cd"
  - lexeme: "abcccccc"
    expected_token: "cd" 
```

#### 模式 3：正则表达式转化
*    调用可执行文件时，可输出到指定目录。
*    输入要处理的正则表达式个数以及具体的表达式。
*    支持的符号：`*`, `|`, `()`, `[]`, `?`, `+`。
*    示例输入：`(a|b)*abb` 或 `[a-z][a-z0-9]*`。
*    自动生成三份可视化结果：
     * `nfa.png`: NFA
     * `dfa.png`: DFA
     * `min_dfa.png`: 最小化 DFA

## 自动化测试

本项目包含自动化验证脚本，用于批量测试正则表达式生成的自动机是否正确。

### 1. 安装测试依赖
验证脚本依赖 `pydot` 来解析生成的 Graphviz 文件：
```bash
pip install pydot
```

### 2. 配置测试脚本
测试脚本为 `test_correctness.sh`。在运行前，请确保其具有执行权限：
```bash
chmod +x test_correctness.sh
```

脚本默认读取 `tests/` 目录下的测试文件。如果您需要测试不同的测试用例集（例如 `test_cases2.txt`），请用文本编辑器打开 `test_correctness.sh` 并修改 `TEST_FILE` 变量：

```bash
# 在 test_correctness.sh 文件中
TEST_FILE="tests/test_cases2.txt"  # <-- 修改这里为您想要测试的文件名
```

### 3. 运行测试
配置完成后，在命令行直接运行脚本即可：
```bash
./test_correctness.sh
```
脚本将自动编译项目（如需），运行测试用例，验证生成的 DFA 行为，并输出通过/失败的统计结果。

### 4. 测试程序
`tests/testcases/` 目录下存在一些 `.py` 文件用于辅助测试：

| 文件名                    | 功能描述                                               |
|:-----------------------|:---------------------------------------------------|
| `gen_testcases.py`     | 自动生成指定数量的随机正则表达式，结果保存在`testcases/test_cases.txt`中。 |
| `test_custom_lexer.py` | 自动化测试自定义 lexer，对给定规则验证输出的 token 类型是否符合预期。          |
| `test_lexer.py`        | 自动化测试预定义 lexer，从`lexer_cases/`目录下加载输入代码片段。         |
| `verify_dot.py`        | 以Python的`re.fullmatch`作为标准，验证由正则表达式生成的 DFA 是否语义正确。 |

## 输出结果

常规运行脚本 `build_and_run.sh` 会自动为每个正则表达式创建一个独立的文件夹（名称经过安全清洗），包含：
*   `nfa_graph.dot`: NFA 的 Graphviz 描述文件。
*   `nfa.png`: NFA 的可视化图片。
*   `dfa_graph.dot`: DFA 的 Graphviz 描述文件。
*   `dfa.png`: DFA 的可视化图片。
*   `min_dfa_graph.dot`: 最小化 DFA 的 Graphviz 描述文件。
*   `min_dfa.png`: 最小化 DFA 的可视化图片。

---
*注意：生成的文件夹名称可能会保留部分特殊字符（如括号），在终端操作时请使用引号包裹路径。*
