############################################################
# The ISIS project
# 
# Main CMake configuration file of the ISIS tools.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 13:47:28 +0200
# 
# The ISIS tools are:
#
# isisdump    -    Dumps the meta data informations from an image file as seen
#                  by the ISIS internal data representation.
# isisiff     -    Shows the  difference between the meta information
#                  given by two image files A and B.
# isisconv    -    The ISIS data converter. It converts image data between all
#                  different formats provided by the imageIO plugins
# isisswap    -    Swaps the image orientation and voxel data along a given axis.

###########################################################
# find and configure the isis core library
###########################################################
include_directories(${CMAKE_SOURCE_DIR}/lib/Core)


############################################################
# configure the targets
############################################################
add_executable(isisdump isisdump.cpp)
target_link_libraries(isisdump isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isisdump PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(isisdiff isisdiff.cpp)
target_link_libraries(isisdiff isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isisdiff PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(isisconv isisconv.cpp)
target_link_libraries(isisconv isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isisconv PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(isisswap isisswap.cpp)
target_link_libraries(isisswap isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isisswap PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

# ###########################################################
# # CPack config
# ###########################################################
# SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "isis cli tools")
# SET(CPACK_PACKAGE_VENDOR "the friendly isis team")
# SET(CPACK_PACKAGE_CONTACT "reimer@cbs.mpg.de")
# SET(CPACK_PACKAGE_NAME "isis-tools")
# SET(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
# #SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
# SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING.txt")
# SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
# SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
# SET(CPACK_PACKAGE_VERSION_PATCH ${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
# SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${ISIS_CORE_REV_STRING}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
# #SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}")
# INCLUDE(CPack)

# the install targets.
install(TARGETS
  isisdump
  isisdiff
  isisconv
  isisswap
  RUNTIME DESTINATION bin)

