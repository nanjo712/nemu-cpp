add_library(
    Utils 
    Disasm.cpp 
)
target_link_libraries(Utils PRIVATE LLVM)
target_include_directories(Utils PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(
    Memory
    Memory.cpp
)
target_link_libraries(Memory PRIVATE fmt spdlog)
target_include_directories(Memory PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(ISA)

add_library(
    Monitor STATIC
    Monitor.cpp
)
target_link_libraries(Monitor PRIVATE fmt spdlog ISA_Wrapper)
target_include_directories(Monitor PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(
    Debugger
    Debugger.cpp
    Expr.cpp
)   
target_link_libraries(Debugger PRIVATE Monitor ISA_Wrapper ${Readline_LIBRARY})
target_include_directories(Debugger PUBLIC ${CMAKE_SOURCE_DIR}/include ${Readline_INCLUDE_DIR})