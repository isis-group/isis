cmake_minimum_required(VERSION 2.4)

project(isis)

#if you don't want the full compiler output, remove the following line
set(CMAKE_VERBOSE_MAKEFILE ON)

#add definitions, compiler switches, etc.
add_definitions(-Wall)


add_subdirectory(lib)
add_subdirectory(tests)
