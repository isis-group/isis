
# this option is required in cmake version 2.4
cmake_minimum_required(VERSION 2.4)

PROJECT(isisCheckerboard3d)
cmake_policy(SET CMP0003 NEW)
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

#the executables
ADD_EXECUTABLE(isischeckerboard3d
	isisCheckerboard3D.cxx
	)
	
set_target_properties(isisdotrans3d PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${ISIS_INSTALL_BIN_DIR}
    COMPILE_DEFINITIONS "SVN_REVISION=${SVN_REVISION}")

# link versus ITK library
TARGET_LINK_LIBRARIES(isischeckerboard3d  ITKIO ITKCommon viaio)