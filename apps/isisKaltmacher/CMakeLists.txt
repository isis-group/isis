
# this option is required in cmake version 2.4
cmake_minimum_required(VERSION 2.4)

PROJECT(isisKaltmacher)

cmake_policy(SET CMP0003 NEW)

#the executables
ADD_EXECUTABLE(isisKaltmacher isisKaltmacher.cxx)

set_target_properties(isisKaltmacher PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ISIS_INSTALL_BIN_DIR}
    COMPILE_DEFINITIONS "SVN_REVISION=${SVN_REVISION}")

# link versus ITK library
TARGET_LINK_LIBRARIES(isisKaltmacher ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio gslcblas)

