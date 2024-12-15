add_executable(test test.cpp)

target_link_libraries(test PRIVATE dtp spdlog::spdlog_header_only)
