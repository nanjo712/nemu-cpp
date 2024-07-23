add_subdirectory(ISA)

add_library(
    Nemu STATIC
    Memory.cpp
    Monitor.cpp
)
target_link_libraries(Nemu PRIVATE fmt spdlog ISA_Wrapper)
target_include_directories(Nemu PUBLIC ${CMAKE_SOURCE_DIR}/include)