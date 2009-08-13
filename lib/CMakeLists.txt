cmake_minimum_required(VERSION 2.4)

# the libISIS project
project(libisis)

# add path for new modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	"${CMAKE_SOURCE_DIR}/cmake/Modules")

include_directories(.)

#add definitions, compiler switches, etc.
add_definitions(-Wall)


add_subdirectory(CoreUtils)
add_subdirectory(DataStorage)
#add_subdirectory(ImageIO)
add_subdirectory(extRegistration)

