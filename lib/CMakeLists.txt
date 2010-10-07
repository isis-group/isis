# the ISIS library module
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(isis_lib)

ADD_SUBDIRECTORY(Core)

# configure_file("../cmake/isis_corecfg.cmake.in" "isis_corecfg.cmake")

# IF(ITK_FOUND)
# 	 add_subdirectory(extITK)
# ENDIF(ITK_FOUND)

# add_subdirectory(Adapter)
