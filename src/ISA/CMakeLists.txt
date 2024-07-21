add_subdirectory(riscv32)

add_library(
    ISA_Wrapper
    ISA_Wrapper.cpp
)
target_include_directories(
    ISA_Wrapper
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)