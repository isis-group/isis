
PROJECT(propertyconcept)

cmake_minimum_required(VERSION 2.4)

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall)

#list all source files here
ADD_EXECUTABLE(propertyconcept main.cpp type.cpp  propmap.cpp)

#need to link to some other libraries ? just add them here
#TARGET_LINK_LIBRARIES(propertyconcept png jpeg)

