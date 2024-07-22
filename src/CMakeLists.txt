add_subdirectory(ISA)

add_library(
    Nemu STATIC
    Memory.cpp
    CPU.cpp
)
target_link_libraries(Nemu PRIVATE spdlog)
target_include_directories(Nemu PUBLIC ${CMAKE_SOURCE_DIR}/include)