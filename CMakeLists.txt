cmake_minimum_required(VERSION 3.18)

project(parser)

add_executable(parser parser.cpp)

set(LLVM_HOME ~/)

target_include_directories(parser PUBLIC ${LLVM_HOME}/build/include)
set_target_properties(parser PROPERTIES CXX_STANDARD 14)

find_library(CLANG_LIB NAMES clang-cpp HINTS ${LLVM_HOME}/build/lib)
find_library(debugLLVM_LIB NAMES LLVM-10 HINTS ${LLVM_HOME}/build/lib NO_DEFAULT_PATH)

target_link_libraries(parser ${CLANG_LIB} ${debugLLVM_LIB})
