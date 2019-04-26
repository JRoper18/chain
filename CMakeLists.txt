cmake_minimum_required(VERSION 3.10.2)
project(compiler)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
add_executable(compiler main.c value.h functions.h t1.c)
add_executable(t0 t0.c value.h functions.h)
add_executable(t1 t1.c value.h functions.h)