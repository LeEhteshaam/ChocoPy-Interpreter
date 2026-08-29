cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ChocoPy)
# Build main interpreter
add_executable(interp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.cpp
)

# Build test runner
add_executable(test_lexer 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_lexer.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.cpp
)