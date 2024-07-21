add_subdirectory(ISA)

add_library(
    Memory STATIC
    Memory.cpp
)
target_link_libraries(Memory PRIVATE spdlog)
target_include_directories(Memory PUBLIC ${CMAKE_SOURCE_DIR}/include)