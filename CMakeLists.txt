cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ChocoPy)

# Build main interpreter
add_executable(interp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ast_printer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/environment.cpp
)

# Build test runner
add_executable(run_tests 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_ast_printer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ast_printer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/environment.cpp
)