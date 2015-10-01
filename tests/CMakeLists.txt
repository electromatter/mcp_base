add_executable(fbuf_test fbuf_test.c)
target_link_libraries(fbuf_test mcp_base)

add_executable(mcp_test mcp_test.c)
target_link_libraries(mcp_test mcp_base)

add_executable(mcg_test mcg_test.c)
target_link_libraries(mcg_test mcp_base)

find_program(CTEST_MEMORYCHECK_COMMAND valgrind)
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")

add_test(NAME fbuf_test COMMAND fbuf_test 0 1 2)
add_test(NAME mcp_test COMMAND mcp_test)
add_test(NAME mcg_test COMMAND mcg_test)
