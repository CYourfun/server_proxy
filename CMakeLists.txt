# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.2)

project ("zzy")

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 将源代码添加到此项目的可执行文件。
add_executable (server "server.c")

if(WIN32)
	target_compile_definitions(server PRIVATE _GNU_SOURCE _WIN32_WINNT=0x0600 _CRT_SECURE_NO_WARNINGS)
	target_compile_options(server PRIVATE /wd4100)
else()
	target_compile_definitions(server PRIVATE _GNU_SOURCE)
	target_compile_options(server PRIVATE -Wall -Wextra)
	target_compile_options(server PRIVATE -Wno-unused-parameter -Wno-strict-aliasing)
endif()
