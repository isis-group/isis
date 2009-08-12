cmake_minimum_required(VERSION 2.4)

project(libisis)

include_directories(.)

#add definitions, compiler switches, etc.
add_definitions(-Wall)


add_subdirectory(CoreUtils)
add_subdirectory(DataStorage)
#add_subdirectory(ImageIO)
add_subdirectory(extRegistration)

