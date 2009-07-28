
PROJECT(log_concept)

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS(-D_ENABLE_FILEIO_LOG=1)

#list all source files here
ADD_EXECUTABLE(log_concept example.cpp log.h message.cpp message.h)

 
