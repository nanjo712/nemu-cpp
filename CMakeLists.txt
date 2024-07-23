cmake_minimum_required(VERSION 3.29)

project(nemu-cpp LANGUAGES CXX C ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# USE O2
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Werror")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Readline REQUIRED)
find_package(LLVM REQUIRED CONFIG)

add_definitions(-DMEMORY_BASE=0x80000000)
add_definitions(-DMEMORY_SIZE=0x8000000)
add_definitions(-DRESET_PC_OFFSET=0x0)

add_subdirectory(src)
add_subdirectory(app)
