

#if you don't want the full compiler output, remove the following line
set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(-D_ENABLE_FILEIO_LOG=1)

#list all source files here
add_library(logger SHARED log.h message.cpp message.h)

 
