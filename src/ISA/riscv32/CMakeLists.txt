add_library(
    ISA_RISCV32
    register.cpp
    instruction.cpp
)
target_include_directories(
    ISA_RISCV32
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

