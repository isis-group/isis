
# this option is required in cmake version 2.4
cmake_minimum_required(VERSION 2.4)

PROJECT(isisKaltmacher)

cmake_policy(SET CMP0003 NEW)

# set(CMAKE_BUILD_TYPE "Debug")

#if you don't want the full compiler output, remove the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

# look for itk package

FIND_PACKAGE(ITK)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR
    "ITK not found. Please set ITK_DIR.")
ENDIF(ITK_FOUND)

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()

#add definitions, compiler switches, etc.
#ADD_DEFINITIONS(-Wall -O2)


#the executables
ADD_EXECUTABLE(isisKaltmacher
	isisKaltmacher.cxx
	)

set_target_properties(isisKaltmacher PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ISIS_INSTALL_BIN_DIR}
    COMPILE_DEFINITIONS "SVN_REVISION=${SVN_REVISION}")

# link versus ITK library
TARGET_LINK_LIBRARIES(isisKaltmacher ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio gslcblas)

