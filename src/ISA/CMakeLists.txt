add_subdirectory(riscv32)

add_library(
    ISA_Wrapper
    ISA_Wrapper.cpp
)
target_include_directories(
    ISA_Wrapper
    PUBLIC
    ${NEMU_CPP_HOME}/include
)
target_link_libraries(
    ISA_Wrapper
    PRIVATE
    ISA_RISCV32
    Memory
    Utils
)