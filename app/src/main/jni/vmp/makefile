# VMP 模块 Makefile
# 适用于 Windows NDK 编译环境

# NDK 路径（根据实际情况修改）
NDK_PATH := C:\Users\ruilo\AppData\Local\Android\Sdk\ndk\27.0.12077973

# 目标架构（根据实际情况修改）
ARCH := arm64-v8a

# Lua 源代码路径
LUA_SRC_PATH := ..\lua

# 编译标志
CFLAGS := -Wall -O2 -I$(LUA_SRC_PATH) -fPIC

# 链接标志
LDFLAGS := -shared

# 目标文件
TARGET := vmp.so

# 源文件
SRC := vmp.c

# Lua 源文件
LUA_SRC := $(LUA_SRC_PATH)\lapi.c $(LUA_SRC_PATH)\lcode.c $(LUA_SRC_PATH)\lctype.c \
           $(LUA_SRC_PATH)\ldebug.c $(LUA_SRC_PATH)\ldo.c $(LUA_SRC_PATH)\ldump.c \
           $(LUA_SRC_PATH)\lfunc.c $(LUA_SRC_PATH)\lgc.c $(LUA_SRC_PATH)\llex.c \
           $(LUA_SRC_PATH)\lmem.c $(LUA_SRC_PATH)\lobject.c $(LUA_SRC_PATH)\lopcodes.c \
           $(LUA_SRC_PATH)\lparser.c $(LUA_SRC_PATH)\lstate.c $(LUA_SRC_PATH)\lstring.c \
           $(LUA_SRC_PATH)\ltable.c $(LUA_SRC_PATH)\ltm.c $(LUA_SRC_PATH)\lundump.c \
           $(LUA_SRC_PATH)\lvm.c $(LUA_SRC_PATH)\lzio.c $(LUA_SRC_PATH)\lauxlib.c \
           $(LUA_SRC_PATH)\lbaselib.c $(LUA_SRC_PATH)\lcorolib.c $(LUA_SRC_PATH)\ldblib.c \
           $(LUA_SRC_PATH)\liolib.c $(LUA_SRC_PATH)\lmathlib.c $(LUA_SRC_PATH)\loslib.c \
           $(LUA_SRC_PATH)\lstrlib.c $(LUA_SRC_PATH)\ltablib.c $(LUA_SRC_PATH)\lutf8lib.c \
           $(LUA_SRC_PATH)\loadlib.c $(LUA_SRC_PATH)\linit.c

# 目标规则
all: $(TARGET)

$(TARGET): $(SRC) $(LUA_SRC)
	$(NDK_PATH)\toolchains\llvm\prebuilt\windows-x86_64\bin\clang.exe $(CFLAGS) $(LDFLAGS) -o $(TARGET) $^

# 清理规则
clean:
	-del /f $(TARGET)

# 伪目标
.PHONY: all clean
