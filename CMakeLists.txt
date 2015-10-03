cmake_minimum_required(VERSION 2.7)
project(mcp_base)

set(CMAKE_C_FLAGS "-std=c99 -Wextra -Wall -pedantic -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer -fPIC")

add_library(mcp_base fbuf.c mcp.c mcg.c)

enable_testing()

include_directories(.)
add_subdirectory(tests/)

