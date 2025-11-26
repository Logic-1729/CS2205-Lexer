# Regex to NFA to DFA Converter

这是一个基于 C++ 实现的正则表达式引擎核心组件演示项目。该项目能够解析正则表达式，将其转换为非确定性有限自动机 (NFA)，并进一步转换为确定性有限自动机 (DFA)。项目旨在展示词法分析生成器的底层原理，并提供可视化的状态转换图。

## 项目原理

本项目遵循经典的词法分析生成器设计原理，主要流程如下：

### 1. 正则表达式预处理 (Preprocessing)
*   **Token化**: 将输入字符串解析为 Token 流，识别操作符（`*`, `|`, `?`, `+`）和操作数。
*   **显式连接符**: 自动在相邻的操作数之间插入显式的连接符 `&`，简化后续解析逻辑。
*   **字符集处理**: 支持 `[...]` 语法，将其解析为字符区间集合（`CharSet`），而非简单的字符串。

### 2. 中缀转后缀 (Infix to Postfix)
*   使用 **Shunting-yard 算法** 将中缀正则表达式转换为后缀表达式（逆波兰表达式），处理操作符优先级（`*` > `+` > `&` > `|`）。

### 3. NFA 构建 (Thompson's Construction)
*   采用 **Thompson 构造法**，通过递归组合小的 NFA 片段构建复杂的 NFA。
*   **智能指针管理**: 使用 `std::shared_ptr` 管理节点内存，避免内存泄漏。
*   支持的操作：
    *   **Union (|)**: 并联。
    *   **Concatenation (&)**: 串联（含节点合并优化）。
    *   **Kleene Star (*)**: 闭包。
    *   **Option (?)**: 零次或一次（语法糖）。
    *   **Plus (+)**: 一次或多次（语法糖）。

### 4. DFA 转换 (Subset Construction)
*   采用 **子集构造法 (Powerset Construction)**。
*   **Epsilon-Closure 缓存**: 优化了闭包计算，使用缓存避免重复遍历，提升性能。
*   **DFA 状态最小化**: （当前版本主要关注构建，未包含 Hopcroft 最小化算法）。

## 代码结构

项目源码位于 `src/` 目录下：

| 文件名 | 功能描述 |
| :--- | :--- |
| `main.cpp` | 程序入口。处理命令行参数，协调解析、构建及输出流程。 |
| `nfa.h` | 定义核心数据结构：`Node`, `Edge`, `NFAUnit`, `CharSet`。 |
| `dfa.h` | 定义 DFA 相关结构：`DFAState`, `DFATransition`。 |
| `regex_parser.h` | 定义解析器接口、Token 结构及异常类 `RegexSyntaxError`。 |
| `regex_preprocessor.cpp` | 实现正则预处理、字符集解析及 Token 流生成。 |
| `infix_to_postfix.cpp` | 实现 Shunting-yard 算法。 |
| `nfa_builder.cpp` | 实现 Thompson 构造法构建 NFA。 |
| `dfa_converter.cpp` | 实现 NFA 到 DFA 的转换逻辑（含闭包缓存）。 |
| `visualize.cpp` | 负责生成 Graphviz `.dot` 文件。 |

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
    脚本会提示输入要处理的正则表达式个数以及具体的表达式。
    *   支持的符号：`*`, `|`, `()`, `[]`, `?`, `+`。
    *   示例输入：`(a|b)*abb` 或 `[a-z][a-z0-9]*`。

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

可执行文件支持指定输出目录：

```bash
./regex_automata "output_directory_path"
```
*   程序会从标准输入读取一个正则表达式。
*   生成的 `nfa_graph.dot` 和 `dfa_graph.dot` 将保存在指定目录中。

## 输出结果

脚本会自动为每个正则表达式创建一个独立的文件夹（名称经过安全清洗），包含：
*   `nfa_graph.dot`: NFA 的 Graphviz 描述文件。
*   `nfa.png`: NFA 的可视化图片。
*   `dfa_graph.dot`: DFA 的 Graphviz 描述文件。
*   `dfa.png`: DFA 的可视化图片。

## Testing

测试脚本位于 `tests/` 目录下。

### 1. 安装依赖

首先安装 Python 依赖包 `pydot`：

```bash
pip install pydot
```

### 2. 运行测试

1.  **进入 `tests` 目录**:
    ```bash
    cd tests
    ```

2.  **赋予脚本执行权限**:
    ```bash
    chmod +x test_correctness.sh
    ```

3.  **运行测试脚本**:
    ```bash
    ./test_correctness.sh
    ```

### 3. 自定义测试用例

您可以修改 `test_correctness.sh` 脚本中的 `TEST_FILE` 变量来测试不同的测试用例文件：

```bash
TEST_FILE="$SCRIPT_DIR/test_cases2.txt"  # 修改为其他测试文件，如 test_cases0.txt
```

`tests/` 目录下提供了多个测试用例文件（`test_cases0.txt` 至 `test_cases9.txt`）供选择。

---
*注意：生成的文件夹名称可能会保留部分特殊字符（如括号），在终端操作时请使用引号包裹路径。*
