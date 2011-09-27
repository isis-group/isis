############################################################
# The ISIS project
# 
# Main CMake configuration file of the ISIS python extension
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 15:01:03 +0200
# 
############################################################

############################################################
# include ISIS core header files
############################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/Core)

find_package(Boost REQUIRED COMPONENTS python)
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_PATH})

add_subdirectory(isis)

