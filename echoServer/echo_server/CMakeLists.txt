cmake_minimum_required(VERSION 2.8.12)

project("echo_server")

add_executable(echo_server
			"main.c"
			"queue.h"
			"ev2.h"
			"ev2.c"
			"echo_server.h"
			"echo_server.c"
			"timerfd.h"
			"timerfd.c")

if(WIN32)
	target_compile_definitions(echo_server PRIVATE _GNU_SOURCE _WIN32_WINNT=0x0600 _CRT_SECURE_NO_WARNINGS)
	target_compile_options(echo_server PRIVATE /wd4100)
else()
	target_compile_definitions(echo_server PRIVATE _GNU_SOURCE)
	target_compile_options(echo_server PRIVATE -Wall -Wextra)
	target_compile_options(echo_server PRIVATE -Wno-unused-parameter -Wno-strict-aliasing)
endif()
