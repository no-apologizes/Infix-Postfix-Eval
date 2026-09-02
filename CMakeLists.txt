cmake_minimum_required(VERSION 3.25)
project(infix_postfix_eval C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(infix_postfix_eval
        main.c
        lexer.c
        parser.c
        eval.c
)

option(QODANA_ANALYSIS "Building for Qodana static analysis, disable LTO/native flags" OFF)

if(QODANA_ANALYSIS)
    target_compile_options(infix_postfix_eval PRIVATE
            -O0 -std=c17
            -Wall -Wextra -Wshadow -Wconversion -Wnull-dereference
    )
else()
    target_compile_options(infix_postfix_eval PRIVATE
            -O3 -march=native -std=c17 -flto
            -Wall -Wextra -Wshadow -Wconversion -Wnull-dereference
    )
endif()


target_include_directories(infix_postfix_eval PRIVATE
        ${CMAKE_SOURCE_DIR}/Headers
)

target_link_libraries(infix_postfix_eval PRIVATE m)
