#!/bin/bash
# 文件名：check_env.sh

echo "=== 环境变量检查 ==="
echo "WORKSPACE: $WORKSPACE"
echo "EDK_TOOLS_PATH: $EDK_TOOLS_PATH"
echo "CONF_PATH: $CONF_PATH"

echo "=== 文件系统检查 ==="
echo "1. EDK II目录是否存在: $(ls -d $WORKSPACE/edk2 2>/dev/null | wc -l)"
echo "2. 包DEC文件是否存在: $(ls $WORKSPACE/MyHttpClientPkg/MyHttpClientPkg.dec 2>/dev/null | wc -l)"
echo "3. 应用程序INF文件是否存在: $(ls $WORKSPACE/MyHttpClientPkg/Applications/HttpPostApp/HttpPostApp.inf 2>/dev/null | wc -l)"
echo "4. target.txt配置:"
cat $WORKSPACE/Conf/target.txt 2>/dev/null || echo "target.txt未找到"

echo "=== 建议的构建命令 ==="
echo "build -p MyHttpClientPkg/MyHttpClientPkg.dsc -a X64 -t GCC5 -b DEBUG"
