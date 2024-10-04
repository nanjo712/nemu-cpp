add_library(
    Utils 
    Disasm.cpp 
    Elf_Parser.cpp
)
target_link_libraries(Utils PRIVATE LLVM)
target_include_directories(Utils PUBLIC ${NEMU_CPP_HOME}/include)

add_library(
    Memory
    Memory.cpp
)
target_link_libraries(Memory PRIVATE spdlog::spdlog_header_only)
target_include_directories(Memory PUBLIC ${NEMU_CPP_HOME}/include)

add_subdirectory(ISA)