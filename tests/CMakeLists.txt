
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ISIS_TESTSUITE)

SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH
    "The directory where the isis cmake configuration skript script is located.")
FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIRS})

# we use the log definitions of the core
IF(ISIS_RUNTIME_LOG)
	ADD_DEFINITIONS(-D_ENABLE_LOG=1)
ELSE(ISIS_RUNTIME_LOG)
	ADD_DEFINITIONS(-D_ENABLE_LOG=0)
ENDIF(ISIS_RUNTIME_LOG)

# enable DEBUG_LOG
IF(ISIS_DEBUG_LOG)
	ADD_DEFINITIONS(-D_ENABLE_DEBUG=1)
	MESSAGE(STATUS "Debug code enabled, this will will have a big impact on the performance")
ELSE(ISIS_DEBUG_LOG)
	ADD_DEFINITIONS(-D_ENABLE_DEBUG=0)
ENDIF(ISIS_DEBUG_LOG)

FIND_PACKAGE(Boost REQUIRED COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(CoreUtils)
add_subdirectory(DataStorage)
add_subdirectory(ImageIO)

# IF(${CMAKE_PROJECT_NAME}_HAVE_ITK)
# 	add_subdirectory(extRegistration)
# ENDIF(${CMAKE_PROJECT_NAME}_HAVE_ITK)
# 
# IF(${CMAKE_PROJECT_NAME}_HAVE_VTK OR ${CMAKE_PROJECT_NAME}_HAVE_ITK)
#     add_subdirectory(ExternalLibraryAdapter)
# ENDIF(${CMAKE_PROJECT_NAME}_HAVE_VTK OR ${CMAKE_PROJECT_NAME}_HAVE_ITK)

# add_executable(test_logger test_logger.cpp)
# target_link_libraries(test_logger utils ${Boost_LIBRARIES})
# 
# add_executable(test_parameter test_parameter.cpp)
# target_link_libraries(test_parameter utils ${Boost_LIBRARIES})
