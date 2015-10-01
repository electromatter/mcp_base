add_executable(fbuf_test fbuf_test.c)
target_link_libraries(fbuf_test mcp_base)

add_test(NAME fbuf_test COMMAND fbuf_test)

