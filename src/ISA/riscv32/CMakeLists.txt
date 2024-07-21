add_library(
    ISA_RISCV32
    Register.cpp
    Instruction.cpp
)
target_include_directories(
    ISA_RISCV32
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

