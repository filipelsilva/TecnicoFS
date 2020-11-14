cmake_minimum_required(VERSION 3.17)
project(TecnicoFS)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-g -ansi -Wall -Wextra -pthread -lm")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

include_directories(.)
include_directories(fs)
include_directories(inputs)

add_executable(TecnicoFS
        fs/operations.c
        fs/operations.h
        fs/state.c
        fs/state.h
        inputs/test1.txt
        inputs/test2.txt
        inputs/test3.txt
        inputs/test4.txt
        inputs/testsof.txt
        main.c
        tecnicofs-api-constants.h)
