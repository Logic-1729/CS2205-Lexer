cmake_minimum_required(VERSION 3.10)
project(CS2205_Lexer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加所有源文件
set(SOURCES
    src/main.cpp
    src/regex_preprocessor.cpp
    src/regex_simplifier.cpp
    src/infix_to_postfix.cpp
    src/nfa_builder.cpp
    src/dfa_converter.cpp
    src/dfa_minimizer.cpp
    src/visualize.cpp
    src/lexer.cpp
)

# 创建可执行文件
add_executable(regex_automata ${SOURCES})

# 包含头文件目录
target_include_directories(regex_automata PRIVATE src)