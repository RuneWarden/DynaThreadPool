cmake_minimum_required(VERSION 3.29.6)

project(dtp CXX)

set(CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "C:/Users/RuneWarden/scoop/apps/vcpkg/2024.10.21/installed/x64-mingw-static")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

include_directories(${INCLUDE_DIR})

add_library(dtp STATIC ${SOURCES})

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(dtp PRIVATE spdlog::spdlog_header_only)

add_subdirectory(test)
