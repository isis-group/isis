# this option is required in cmake version 2.4
cmake_minimum_required(VERSION 2.4)

PROJECT(isisReg3D)

cmake_policy(SET CMP0003 NEW)

# set(CMAKE_BUILD_TYPE "Debug")

#the executables
ADD_EXECUTABLE(isisreg3d isisreg3D.cxx)

set_target_properties(isisreg3d PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ISIS_INSTALL_BIN_DIR}
    COMPILE_DEFINITIONS "SVN_REVISION=${SVN_REVISION}")

# link versus ITK library
TARGET_LINK_LIBRARIES(isisreg3d  ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio utils gslcblas)
