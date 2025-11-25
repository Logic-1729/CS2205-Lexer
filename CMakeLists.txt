cmake_minimum_required(VERSION 3.10)
project(RegexAutomata)

# 设置 C++ 标准为 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(src)

# 定义源文件
set(SOURCES
    src/main.cpp
    src/regex_preprocessor.cpp
    src/infix_to_postfix.cpp
    src/nfa_builder.cpp
    src/dfa_converter.cpp
    src/dfa_minimizer.cpp  # 新增
    src/visualize.cpp
)

# 创建可执行文件
add_executable(regex_automata ${SOURCES})