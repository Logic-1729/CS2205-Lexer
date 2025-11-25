# Regex to NFA to DFA Converter

这是一个基于 C++ 实现的正则表达式引擎核心组件演示项目。该项目能够解析正则表达式，将其转换为非确定性有限自动机 (NFA)，并进一步转换为确定性有限自动机 (DFA)。项目还支持生成 Graphviz DOT 文件以可视化状态机结构。

## 项目原理

本项目遵循经典的词法分析生成器设计原理，参考了 [Charlee Li 的正则表达式解析](https://cn.charlee.li/parse-regex-with-dfa.html) 相关理论。主要流程如下：

### 1. 正则表达式预处理 (Preprocessing)
*   **显式连接符**: 正则表达式中的连接通常是隐式的（如 `ab`）。为了便于后续计算，程序会在相邻的符号之间插入显式的连接符 `+`（例如将 `ab` 转换为 `a+b`）。
*   **字符集处理**: 将 `[a-z]` 或 `[_a-zA-Z]` 等字符集识别为一个完整的 Token 单元，而不是将其拆解为多个字符。这简化了 NFA 的图形结构，使其更易于阅读。

### 2. 中缀转后缀 (Infix to Postfix)
*   使用 **Shunting-yard 算法** 将中缀正则表达式（如 `a+b*`）转换为后缀表达式（逆波兰表达式，如 `ab*+`），以便于通过堆栈进行自动机构建。

### 3. NFA 构建 (Thompson's Construction)
*   采用 **Thompson 构造法**。通过递归地组合小的 NFA 片段来构建复杂的 NFA。
*   **优化**: 本项目在处理 **连接 (Concatenation)** 操作时进行了优化。与标准的 Thompson 构造（增加一条中间 $\epsilon$ 边）不同，本项目直接合并前一个 NFA 的终态和后一个 NFA 的初态。这显著减少了生成的节点数和 $\epsilon$ 边。
*   支持的操作包括：
    *   **Union (|)**: 并联两个 NFA。
    *   **Concatenation (+)**: 串联两个 NFA（节点合并优化）。
    *   **Kleene Star (*)**: 闭包操作。

### 4. DFA 转换 (Subset Construction)
*   采用 **子集构造法 (Powerset Construction)**。
*   **Epsilon-Closure**: 计算从某个状态出发，仅通过 $\epsilon$ 边能到达的所有状态集合。
*   **DFA 状态生成**: DFA 的每一个状态对应 NFA 的一个状态子集，从而消除非确定性。

## 代码结构

项目源码位于 `src/` 目录下，主要文件功能如下：

| 文件名 | 功能描述 |
| :--- | :--- |
| `main.cpp` | 程序入口。负责协调预处理、NFA构建、DFA转换及可视化的整体流程。 |
| `nfa_dfa_builder.h` | 头文件。定义了 `Node`, `Edge`, `NFAUnit`, `DFAState` 等核心数据结构及函数声明。 |
| `regex_preprocessor.cpp` | 负责正则表达式的清洗、Token化（处理 `[...]`）以及插入显式连接符 `+`。 |
| `infix_to_postfix.cpp` | 实现将中缀正则表达式列表转换为后缀表达式的逻辑。 |
| `nfa_builder.cpp` | 实现了基于后缀表达式构建 NFA 的核心逻辑，包含节点合并优化。 |
| `dfa_converter.cpp` | 实现了从 NFA 到 DFA 的转换逻辑 (子集构造法、Epsilon闭包计算)。 |
| `visualize.cpp` | 负责将生成的 NFA 和 DFA 数据结构输出为 Graphviz `.dot` 文件格式。 |
| `build_and_run.sh` | 自动化构建脚本，用于编译代码并执行，同时自动调用 `dot` 命令生成图片。 |

## 环境配置

在运行本项目之前，请确保您的环境满足以下要求：

### 1. 操作系统
*   **Linux / macOS**: 推荐环境，直接支持 shell 脚本。
*   **Windows**: 建议使用 WSL (Windows Subsystem for Linux) 或 MinGW/Cygwin 环境。

### 2. 编译器
需要支持 **C++17** 标准的编译器。
*   `g++` (GCC) 7.0 或更高版本。

### 3. 可视化工具 (Graphviz)
为了生成 `.png` 图片，需要安装 Graphviz。

*   **Ubuntu/Debian**: `sudo apt-get install graphviz`
*   **macOS**: `brew install graphviz`
*   **Windows**: 请从 Graphviz 官网下载安装包，并将 `dot` 命令添加到系统 PATH 环境变量中。

## 使用说明

项目根目录提供了一个脚本，可完成编译、运行和图片生成的一站式操作。

1.  **赋予脚本执行权限**:
    ```bash
    chmod +x build_and_run.sh
    ```

2.  **运行脚本**:
    ```bash
    ./build_and_run.sh
    ```

3.  **输入正则表达式**:
    程序启动后，会提示输入正则表达式。支持的符号包括 `( )`, `*`, `|`, `[...]`。
    
    **输入示例**:
    ```text
    (if)|[_a-zA-Z][_0-9a-zA-Z]*
    ```

4.  **查看结果**:
    *   终端将输出 NFA 和 DFA 的状态转换表文本。
    *   根目录下会自动生成以下文件：
        *   `nfa_graph.dot` & `nfa.png`: NFA 的结构图。
        *   `dfa_graph.dot` & `dfa.png`: DFA 的结构图。