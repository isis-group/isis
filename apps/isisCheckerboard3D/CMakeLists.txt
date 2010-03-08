
# this option is required in cmake version 2.4
cmake_minimum_required(VERSION 2.4)

PROJECT(isisCheckerboard3d)
cmake_policy(SET CMP0003 NEW)

#the executables
ADD_EXECUTABLE(isischeckerboard3d isisCheckerboard3D.cxx)
	
set_target_properties(isischeckerboard3d PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${ISIS_INSTALL_BIN_DIR}
    COMPILE_DEFINITIONS "SVN_REVISION=${SVN_REVISION}")

# link versus ITK library
TARGET_LINK_LIBRARIES(isischeckerboard3d  ITKIO ITKCommon viaio gslcblas)