cmake_minimum_required(VERSION 3.10)
project(lab3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(child1 src/child1.c)
add_executable(child2 src/child2.c)
add_executable(main   src/main.c)