add_library(
    ISA_RISCV32
    EmuCore.cpp
)
target_include_directories(
    ISA_RISCV32
    PUBLIC
    ${NEMU_CPP_HOME}/include
)

