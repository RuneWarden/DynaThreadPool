cmake_minimum_required(VERSION 3.29.6)

project(ThreadPool)

set(CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "C:/Users/RuneWarden/scoop/apps/vcpkg/2024.10.21/installed/x64-mingw-dynamic")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES ${SOURCE_DIR}/*.cpp) 

include_directories(${INCLUDE_DIR})

find_package(spdlog REQUIRED)

add_executable(ThreadPool ${SOURCES})

target_link_libraries(ThreadPool PRIVATE spdlog::spdlog)

file(GLOB VCPKG_BIN_DIR ${CMAKE_PREFIX_PATH}/debug/bin/*.dll)

add_custom_command(TARGET ThreadPool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${VCPKG_BIN_DIR} $<TARGET_FILE_DIR:ThreadPool>)
