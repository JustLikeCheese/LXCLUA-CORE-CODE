# 编码规范 / Style Guide

## 概述 / Overview

本编码规范旨在确保LXCLUA项目的代码质量和一致性，使代码更易于阅读、理解和维护。所有贡献者都应该遵循这些规范，以保持项目的整体代码风格统一。

This style guide aims to ensure code quality and consistency in the LXCLUA project, making code easier to read, understand, and maintain. All contributors should follow these guidelines to maintain a consistent code style throughout the project.

## 通用规范 / General Guidelines

### 1. 缩进与空格 / Indentation and Whitespace

- **缩进**：使用4个空格进行缩进，不使用制表符（Tab）
- **行宽**：每行代码不超过120个字符
- **空行**：在逻辑块之间使用空行分隔，提高代码可读性
- **空格**：在运算符两侧、逗号后、括号内侧等适当位置使用空格

- **Indentation**: Use 4 spaces for indentation, not tabs
- **Line Length**: Each line of code should not exceed 120 characters
- **Blank Lines**: Use blank lines to separate logical blocks, improving code readability
- **Spaces**: Use spaces around operators, after commas, inside parentheses, etc.

### 2. 命名规范 / Naming Conventions

- **变量名**：使用小写字母，单词之间用下划线分隔（snake_case）
- **函数名**：使用小写字母，单词之间用下划线分隔（snake_case）
- **类名**：使用驼峰命名法，首字母大写（PascalCase）
- **常量名**：使用全大写字母，单词之间用下划线分隔（UPPER_SNAKE_CASE）
- **文件名**：使用小写字母，单词之间用下划线分隔，与文件内容的主要功能或类名对应

- **Variable Names**: Use lowercase letters, separate words with underscores (snake_case)
- **Function Names**: Use lowercase letters, separate words with underscores (snake_case)
- **Class Names**: Use camelCase with the first letter capitalized (PascalCase)
- **Constant Names**: Use all uppercase letters, separate words with underscores (UPPER_SNAKE_CASE)
- **File Names**: Use lowercase letters, separate words with underscores, corresponding to the main function or class name in the file

### 3. 注释规范 / Comment Guidelines

- **函数注释**：所有函数都应该有详细的注释，包括功能描述、参数说明和返回值说明
- **代码注释**：对于复杂的代码逻辑，应该添加注释说明
- **TODO注释**：使用TODO注释标记待完成的任务
- **FIXME注释**：使用FIXME注释标记需要修复的问题

- **Function Comments**: All functions should have detailed comments, including function description, parameter explanations, and return value explanations
- **Code Comments**: For complex code logic, add comments to explain
- **TODO Comments**: Use TODO comments to mark tasks to be completed
- **FIXME Comments**: Use FIXME comments to mark issues that need to be fixed

### 4. 代码组织 / Code Organization

- **文件结构**：每个文件应该只包含一个主要功能或类
- **导入语句**：按字母顺序组织导入语句，分组导入（标准库、第三方库、本地库）
- **函数顺序**：按照逻辑顺序组织函数，相关函数放在一起
- **代码块**：使用大括号包围代码块，即使只有一行代码

- **File Structure**: Each file should contain only one main function or class
- **Import Statements**: Organize import statements alphabetically, grouping imports (standard libraries, third-party libraries, local libraries)
- **Function Order**: Organize functions in logical order, placing related functions together
- **Code Blocks**: Use braces to enclose code blocks, even for single-line code

### 5. 错误处理 / Error Handling

- **异常处理**：适当使用异常处理，不要捕获所有异常
- **错误信息**：提供清晰、详细的错误信息
- **资源管理**：确保资源正确释放，避免资源泄漏

- **Exception Handling**: Use exception handling appropriately, don't catch all exceptions
- **Error Messages**: Provide clear, detailed error messages
- **Resource Management**: Ensure resources are properly released, avoid resource leaks

## 语言特定规范 / Language-Specific Guidelines

### 1. Java/Kotlin 规范 / Java/Kotlin Guidelines

#### Java 规范 / Java Guidelines

- **包名**：使用小写字母，域名反转，例如 `com.difierline.lua`
- **类名**：使用PascalCase命名法
- **方法名**：使用驼峰命名法，首字母小写（camelCase）
- **变量名**：使用驼峰命名法，首字母小写（camelCase）
- **常量**：使用全大写字母，单词之间用下划线分隔
- **注释**：使用Javadoc格式注释类和方法
- **代码风格**：遵循Google Java Style Guide

- **Package Names**: Use lowercase letters, reversed domain name, e.g., `com.difierline.lua`
- **Class Names**: Use PascalCase naming convention
- **Method Names**: Use camelCase naming convention, lowercase first letter
- **Variable Names**: Use camelCase naming convention, lowercase first letter
- **Constants**: Use all uppercase letters, separate words with underscores
- **Comments**: Use Javadoc format to comment classes and methods
- **Code Style**: Follow Google Java Style Guide

#### Kotlin 规范 / Kotlin Guidelines

- **包名**：与Java相同，使用小写字母，域名反转
- **类名**：使用PascalCase命名法
- **函数名**：使用驼峰命名法，首字母小写（camelCase）
- **变量名**：使用驼峰命名法，首字母小写（camelCase）
- **常量**：使用全大写字母，单词之间用下划线分隔
- **注释**：使用KDoc格式注释类和方法
- **代码风格**：遵循Kotlin官方编码规范
- **空安全**：正确使用Kotlin的空安全特性

- **Package Names**: Same as Java, use lowercase letters, reversed domain name
- **Class Names**: Use PascalCase naming convention
- **Function Names**: Use camelCase naming convention, lowercase first letter
- **Variable Names**: Use camelCase naming convention, lowercase first letter
- **Constants**: Use all uppercase letters, separate words with underscores
- **Comments**: Use KDoc format to comment classes and methods
- **Code Style**: Follow Kotlin official coding conventions
- **Null Safety**: Correctly use Kotlin's null safety features

### 2. C/C++ 规范 / C/C++ Guidelines

- **头文件**：使用`#ifndef`/`#define`/`#endif`防止头文件重复包含
- **命名**：使用小写字母，单词之间用下划线分隔
- **类型定义**：使用`typedef`为复杂类型创建别名时，在名称末尾添加`_t`
- **指针**：星号靠近变量名，例如 `int *ptr` 而不是 `int* ptr`
- **注释**：使用`/* */`进行块注释，`//`进行行注释
- **代码风格**：遵循Linux内核编码风格

- **Header Files**: Use `#ifndef`/`#define`/`#endif` to prevent duplicate header file inclusion
- **Naming**: Use lowercase letters, separate words with underscores
- **Type Definitions**: When using `typedef` to create aliases for complex types, add `_t` at the end of the name
- **Pointers**: Asterisk close to variable name, e.g., `int *ptr` instead of `int* ptr`
- **Comments**: Use `/* */` for block comments, `//` for line comments
- **Code Style**: Follow Linux kernel coding style

### 3. Lua 规范 / Lua Guidelines

- **命名**：使用小写字母，单词之间用下划线分隔
- **函数**：使用`local`关键字定义局部函数，避免全局函数
- **变量**：优先使用局部变量，避免全局变量
- **表结构**：使用缩进和换行使表结构更清晰
- **注释**：使用`--`进行单行注释，`--[[ ]]`进行块注释
- **代码风格**：遵循Lua官方风格指南

- **Naming**: Use lowercase letters, separate words with underscores
- **Functions**: Use `local` keyword to define local functions, avoid global functions
- **Variables**: Prefer local variables, avoid global variables
- **Table Structures**: Use indentation and line breaks to make table structures clearer
- **Comments**: Use `--` for single-line comments, `--[[ ]]` for block comments
- **Code Style**: Follow Lua official style guide

## 提交消息规范 / Commit Message Guidelines

- **格式**：`<类型>: <主题>`，例如 `feat: 添加网络请求功能`
- **类型**：
  - `feat`: 新功能
  - `fix`: 修复bug
  - `docs`: 文档更改
  - `style`: 代码风格更改（不影响功能）
  - `refactor`: 代码重构（不添加功能或修复bug）
  - `test`: 添加或修改测试
  - `chore`: 构建过程或辅助工具的更改
- **主题**：简短描述（不超过50个字符）
- **正文**：详细描述更改的内容、原因和影响（可选）
- ** footer**：引用相关issue或PR（可选）

- **Format**: `<type>: <subject>`, e.g., `feat: add network request functionality`
- **Types**:
  - `feat`: New feature
  - `fix`: Bug fix
  - `docs`: Documentation changes
  - `style`: Code style changes (no functional impact)
  - `refactor`: Code refactoring (no new features or bug fixes)
  - `test`: Add or modify tests
  - `chore`: Changes to build process or auxiliary tools
- **Subject**: Short description (no more than 50 characters)
- **Body**: Detailed description of the changes, reasons, and impact (optional)
- **Footer**: Reference related issues or PRs (optional)

## 文档编写规范 / Documentation Guidelines

- **语言**：文档应该同时支持中英文双语
- **格式**：遵循GitHub Markdown规范
- **结构**：使用清晰的标题层级，组织文档结构
- **示例**：为代码和功能提供示例
- **更新**：确保文档与代码保持同步，及时更新
- **链接**：使用相对链接引用项目内的其他文档

- **Language**: Documentation should support both Chinese and English
- **Format**: Follow GitHub Markdown specifications
- **Structure**: Use clear heading levels to organize document structure
- **Examples**: Provide examples for code and functionality
- **Updates**: Ensure documentation stays in sync with code, update promptly
- **Links**: Use relative links to reference other documents within the project

## 代码审查规范 / Code Review Guidelines

### 审查重点 / Review Focus

- **代码质量**：代码是否符合编码规范，是否易于理解和维护
- **功能正确性**：代码是否正确实现了预期功能
- **性能**：代码是否具有良好的性能
- **安全性**：代码是否存在安全漏洞
- **测试覆盖**：代码是否有足够的测试覆盖
- **文档完整性**：是否有完整的文档

- **Code Quality**: Does the code meet coding standards, is it easy to understand and maintain
- **Functional Correctness**: Does the code correctly implement the intended functionality
- **Performance**: Does the code have good performance
- **Security**: Are there any security vulnerabilities in the code
- **Test Coverage**: Does the code have sufficient test coverage
- **Documentation Completeness**: Is documentation complete

### 审查流程 / Review Process

1. **初步审查**：检查代码是否符合编码规范，是否有明显的问题
2. **详细审查**：深入审查代码逻辑，确保功能正确性和性能
3. **测试验证**：确保代码通过测试
4. **反馈建议**：提供具体的反馈和改进建议
5. **最终批准**：代码符合所有要求后批准合并

1. **Initial Review**: Check if code meets coding standards, if there are any obvious issues
2. **Detailed Review**: Deeply review code logic, ensure functional correctness and performance
3. **Test Verification**: Ensure code passes tests
4. **Feedback Suggestions**: Provide specific feedback and improvement suggestions
5. **Final Approval**: Approve merge after code meets all requirements

### 审查反馈 / Review Feedback

- **积极反馈**：肯定代码中的优点和良好实践
- **具体建议**：提供具体的改进建议，避免模糊的批评
- **尊重态度**：以尊重和建设性的态度提供反馈
- **解释原因**：解释建议的原因，帮助开发者理解

- **Positive Feedback**: Acknowledge strengths and good practices in the code
- **Specific Suggestions**: Provide specific improvement suggestions, avoid vague criticism
- **Respectful Attitude**: Provide feedback in a respectful and constructive manner
- **Explain Reasons**: Explain the reasons for suggestions, help developers understand

## 工具与自动化 / Tools and Automation

### 代码格式化工具 / Code Formatting Tools

- **Java/Kotlin**: 使用Android Studio的内置格式化工具
- **C/C++**: 使用clang-format
- **Lua**: 使用lua-fmt

- **Java/Kotlin**: Use Android Studio's built-in formatting tools
- **C/C++**: Use clang-format
- **Lua**: Use lua-fmt

### 静态代码分析 / Static Code Analysis

- **Java/Kotlin**: 使用Android Lint
- **C/C++**: 使用clang-tidy
- **Lua**: 使用luacheck

- **Java/Kotlin**: Use Android Lint
- **C/C++**: Use clang-tidy
- **Lua**: Use luacheck

### 测试工具 / Testing Tools

- **单元测试**: 使用JUnit (Java)、KotlinTest (Kotlin)、LuaUnit (Lua)
- **集成测试**: 使用Espresso (Android UI测试)

- **Unit Testing**: Use JUnit (Java), KotlinTest (Kotlin), LuaUnit (Lua)
- **Integration Testing**: Use Espresso (Android UI testing)

## 总结 / Summary

遵循这些编码规范将有助于保持LXCLUA项目的代码质量和一致性，使代码更易于阅读、理解和维护。所有贡献者都应该熟悉并遵循这些规范，以确保项目的长期健康发展。

Following these coding guidelines will help maintain code quality and consistency in the LXCLUA project, making code easier to read, understand, and maintain. All contributors should be familiar with and follow these guidelines to ensure the long-term healthy development of the project.

如果您对这些规范有任何疑问或建议，请联系项目维护者。

If you have any questions or suggestions about these guidelines, please contact the project maintainers.

---

© 2026 DifierLine. All rights reserved.
