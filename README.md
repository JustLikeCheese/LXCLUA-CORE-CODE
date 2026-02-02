# LXCLUA

## 项目概述 / Project Overview

LXCLUA是一个功能强大的Lua脚本运行环境，专为Android平台设计。它提供了丰富的API和扩展库，使开发者能够使用Lua语言快速开发Android应用和工具。

LXCLUA is a powerful Lua script runtime environment designed for the Android platform. It provides rich APIs and extension libraries, enabling developers to quickly develop Android applications and tools using the Lua language.

### 主要特点 / Key Features

- **丰富的Lua扩展库**：集成了多种Lua扩展库，包括网络、数据库、加密等
- **Native接口**：提供了与Android系统交互的原生接口
- **高性能**：使用NDK编译，提供高性能的脚本执行环境
- **模块化设计**：采用模块化架构，易于扩展和维护
- **多语言支持**：内置中英文语言包
- **强大的文件系统访问**：支持完整的文件系统操作
- **网络功能**：支持HTTP、Socket等网络操作
- **传感器支持**：可以访问设备传感器数据
- **图形绘制**：支持Canvas绘图功能

- **Rich Lua Extension Libraries**: Integrated with various Lua extension libraries, including network, database, encryption, etc.
- **Native Interface**: Provides native interfaces for interacting with the Android system
- **High Performance**: Compiled with NDK, providing a high-performance script execution environment
- **Modular Design**: Adopts a modular architecture, easy to extend and maintain
- **Multi-language Support**: Built-in Chinese and English language packs
- **Powerful File System Access**: Supports complete file system operations
- **Network Functionality**: Supports HTTP, Socket and other network operations
- **Sensor Support**: Can access device sensor data
- **Graphic Drawing**: Supports Canvas drawing functionality

## 项目架构 / Project Architecture

### 目录结构 / Directory Structure

```
LXCLUA/
├── app/
│   ├── libs/              # 第三方库
│   ├── src/main/
│   │   ├── assets/        # 资源文件
│   │   │   ├── lua/       # Lua脚本
│   │   │   ├── res/       # 资源文件
│   │   │   └── keys/      # 密钥文件
│   │   ├── jni/           # 原生代码
│   │   │   ├── lua/       # Lua核心库
│   │   │   ├── LuaBoost/  # Lua增强库
│   │   │   ├── MMKV/      # 键值存储库
│   │   │   ├── bson/      # BSON库
│   │   │   ├── canvas/    # 绘图库
│   │   │   ├── cjson/     # JSON库
│   │   │   ├── crypt/     # 加密库
│   │   │   ├── ffi/       # FFI库
│   │   │   ├── network/   # 网络库
│   │   │   └── ...        # 其他扩展库
│   │   ├── res/           # Android资源
│   │   └── resources/     # 其他资源
│   ├── AndroidManifest.xml # 应用配置
│   └── build.gradle.kts   # 构建配置
├── color-picker/          # 颜色选择器模块
└── .gitignore             # Git忽略文件
```

### 核心组件 / Core Components

1. **Lua核心**：基于标准Lua实现，提供基本的脚本执行环境
2. **扩展库**：丰富的Lua扩展库，增强了Lua的功能
3. **Native接口**：通过JNI实现的原生接口，连接Lua和Android系统
4. **应用框架**：提供应用生命周期管理和UI交互
5. **资源管理**：管理应用的资源文件和配置

1. **Lua Core**: Based on standard Lua implementation, providing basic script execution environment
2. **Extension Libraries**: Rich Lua extension libraries that enhance Lua's functionality
3. **Native Interface**: Native interfaces implemented through JNI, connecting Lua and Android system
4. **Application Framework**: Provides application lifecycle management and UI interaction
5. **Resource Management**: Manages application resource files and configurations

## 技术实现 / Technical Implementation

### 开发环境 / Development Environment

- **Android SDK**: API 36
- **NDK Version**: 27.0.12077973
- **Kotlin**: 2.3.0
- **Java**: 17
- **Minimum SDK**: 26
- **Target SDK**: 33

### 核心技术 / Core Technologies

1. **NDK编译**：使用NDK编译Lua和扩展库，提高执行效率
2. **JNI桥接**：通过JNI实现Lua与Java/Android的交互
3. **模块化设计**：采用模块化架构，便于扩展和维护
4. **多线程处理**：支持多线程执行Lua脚本
5. **文件系统访问**：通过原生接口实现完整的文件系统操作

1. **NDK Compilation**: Compiles Lua and extension libraries using NDK for improved execution efficiency
2. **JNI Bridging**: Implements interaction between Lua and Java/Android through JNI
3. **Modular Design**: Adopts a modular architecture for easy extension and maintenance
4. **Multi-thread Processing**: Supports multi-threaded execution of Lua scripts
5. **File System Access**: Implements complete file system operations through native interfaces

### 主要依赖 / Main Dependencies

- **Lua 5.5**: 核心脚本引擎
- **MMKV**: 高性能键值存储
- **OkHttp**: 网络请求库
- **Glide**: 图片加载库
- **Material UI**: UI组件库
- **Rhino**: JavaScript引擎

- **Lua 5.5**: Core script engine
- **MMKV**: High-performance key-value storage
- **OkHttp**: Network request library
- **Glide**: Image loading library
- **Material UI**: UI component library
- **Rhino**: JavaScript engine

## 功能特性 / Functional Features

### Lua扩展库 / Lua Extension Libraries

- **网络库**：支持HTTP请求、Socket通信
- **数据库**：支持SQLite数据库操作
- **加密库**：支持各种加密算法
- **JSON处理**：支持JSON解析和生成
- **文件系统**：支持完整的文件操作
- **网络库**：支持HTTP请求、Socket通信
- **传感器**：访问设备传感器数据
- **Canvas**：2D绘图功能
- **MMKV**：高性能键值存储
- **系统接口**：访问Android系统功能

- **Network Library**: Supports HTTP requests, Socket communication
- **Database**: Supports SQLite database operations
- **Encryption Library**: Supports various encryption algorithms
- **JSON Processing**: Supports JSON parsing and generation
- **File System**: Supports complete file operations
- **Sensor**: Access device sensor data
- **Canvas**: 2D drawing functionality
- **MMKV**: High-performance key-value storage
- **System Interface**: Access Android system functions

### 应用功能 / Application Features

- **脚本编辑器**：内置Lua脚本编辑器
- **脚本运行**：直接运行Lua脚本
- **多语言支持**：内置中英文语言包
- **夜间模式**：支持夜间显示模式
- **文件管理**：浏览和管理文件系统
- **网络请求**：发送HTTP请求
- **数据库管理**：管理SQLite数据库

- **Script Editor**: Built-in Lua script editor
- **Script Execution**: Directly run Lua scripts
- **Multi-language Support**: Built-in Chinese and English language packs
- **Night Mode**: Supports night display mode
- **File Management**: Browse and manage file system
- **Network Requests**: Send HTTP requests
- **Database Management**: Manage SQLite databases

## 安装指南 / Installation Guide

### 前提条件 / Prerequisites

- Android Studio 2023.1.1 or later
- Android SDK API 36
- NDK 27.0.12077973
- Kotlin 2.3.0
- Java 17

### 构建步骤 / Build Steps

1. **克隆仓库**：`git clone https://github.com/yourusername/LXCLUA.git`
2. **打开项目**：在Android Studio中打开项目
3. **同步依赖**：等待Gradle同步完成
4. **构建项目**：点击"Build > Build Bundle(s) / APK(s) > Build APK(s)"
5. **安装应用**：将生成的APK安装到设备上

1. **Clone the repository**: `git clone https://github.com/yourusername/LXCLUA.git`
2. **Open the project**: Open the project in Android Studio
3. **Sync dependencies**: Wait for Gradle sync to complete
4. **Build the project**: Click "Build > Build Bundle(s) / APK(s) > Build APK(s)"
5. **Install the app**: Install the generated APK to your device

### 配置选项 / Configuration Options

- **NDK版本**：在`app/build.gradle.kts`中配置NDK版本
- **ABI过滤器**：在`app/build.gradle.kts`中配置支持的ABI
- **权限配置**：在`AndroidManifest.xml`中配置应用权限

- **NDK Version**: Configure NDK version in `app/build.gradle.kts`
- **ABI Filters**: Configure supported ABIs in `app/build.gradle.kts`
- **Permission Configuration**: Configure app permissions in `AndroidManifest.xml`

## 使用指南 / Usage Guide

### 基本用法 / Basic Usage

1. **创建脚本**：在`assets/lua/`目录下创建Lua脚本文件
2. **编写代码**：使用Lua语言编写脚本代码
3. **运行脚本**：通过应用界面或调用API运行脚本

1. **Create a script**: Create a Lua script file in the `assets/lua/` directory
2. **Write code**: Write script code using the Lua language
3. **Run the script**: Run the script through the app interface or by calling the API

### 示例代码 / Sample Code

项目中有大量示例代码

The project contains a large number of sample code snippets

### API文档 / API Documentation

详细的API文档请参考项目中的文档目录或在线文档。

For detailed API documentation, please refer to the documentation directory in the project or online documentation.

## 开发指南 / Development Guide

### 项目结构 / Project Structure

项目采用标准的Android项目结构，主要代码位于`app/src/main/`目录下。原生代码位于`app/src/main/jni/`目录，Lua脚本位于`app/src/main/assets/lua/`目录。

The project adopts a standard Android project structure, with main code located in the `app/src/main/` directory. Native code is located in the `app/src/main/jni/` directory, and Lua scripts are located in the `app/src/main/assets/lua/` directory.

### 添加新功能 / Adding New Features

1. **添加Lua扩展**：在`app/src/main/jni/`目录下创建新的扩展模块
2. **注册扩展**：在`lua/linit.c`中注册新的扩展模块
3. **更新构建配置**：在`Android.mk`中添加新的模块
4. **测试功能**：编写测试脚本验证新功能

1. **Add Lua Extension**: Create a new extension module in the `app/src/main/jni/` directory
2. **Register Extension**: Register the new extension module in `lua/linit.c`
3. **Update Build Configuration**: Add the new module in `Android.mk`
4. **Test Functionality**: Write test scripts to verify the new functionality

### 调试技巧 / Debugging Tips

- **日志输出**：使用`print()`函数输出日志
- **错误处理**：使用`pcall()`捕获和处理错误
- **调试器**：使用Lua的调试库进行调试
- **NDK调试**：使用Android Studio的NDK调试功能

- **Log Output**: Use `print()` function to output logs
- **Error Handling**: Use `pcall()` to catch and handle errors
- **Debugger**: Use Lua's debug library for debugging
- **NDK Debugging**: Use Android Studio's NDK debugging functionality

## 贡献指南 / Contribution Guide

我们欢迎社区贡献！请参考[CONTRIBUTING.md](CONTRIBUTING.md)文件了解如何贡献代码。

We welcome community contributions! Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) file to learn how to contribute code.

## 行为准则 / Code of Conduct

请遵守我们的行为准则，详见[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)文件。

Please adhere to our code of conduct,详见[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) file.

## 编码规范 / Coding Standards

请遵循我们的编码规范，详见[STYLE_GUIDE.md](STYLE_GUIDE.md)文件。

Please follow our coding standards,详见[STYLE_GUIDE.md](STYLE_GUIDE.md) file.

## 许可证 / License

本项目采用MIT许可证。详见[LICENSE](LICENSE)文件。

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 联系我们 / Contact Us

- **项目地址**：https://github.com/OxenFxc/LXCLUA-CORE-CODE
- **邮箱**：difierline@yeah.net

- **Project Address**: https://github.com/OxenFxc/LXCLUA-CORE-CODE
- **Email**: difierline@yeah.net

---

© 2026 DifierLine. All rights reserved.
