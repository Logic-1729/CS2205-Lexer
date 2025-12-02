#pragma once

#include "regex_parser.h"
#include <vector>

/**
 * 简化正则表达式：将导出语法转换为基本语法
 * 
 * 导出语法包括：
 * - ? r (可选) -> (r|ε)
 * - r+ (一次或多次) -> rr*
 * - "abc" (字符串) -> a&b&c (已在预处理阶段展开)
 * 
 * 简化后只包含：字符集、空串(ε)、*、|、连接(&)
 */
std::vector<Token> simplifyRegex(const std::vector<Token>& tokens);

/**
 * 验证是否为简化正则表达式
 */
bool isSimplified(const std::vector<Token>& tokens);