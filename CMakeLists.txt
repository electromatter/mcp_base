cmake_minimum_required(VERSION 2.7)
project(mcp_base)

set(CMAKE_C_FLAGS "-std=c99 -Wextra -Wall -pedantic -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer -fPIC")

include_directories(include/)
add_library(mcp_base fbuf.c mcp.c mcg.c)

include(CTest)

#enable_testing()

if(BUILD_TESTING)
	add_subdirectory(tests/)
endif()

