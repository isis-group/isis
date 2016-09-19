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
# isisflip    -    Flips the image orientation and voxel data along a given axis.

###########################################################
# find and configure the isis core library
###########################################################
include_directories(${CMAKE_SOURCE_DIR})

############################################################
# configure the targets
############################################################
add_executable(isisdump isisdump.cpp)
target_link_libraries(isisdump ${isis_core_lib})
set_target_properties(isisdump PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisdump "print out the metadata of an image")

add_executable(isisdiff isisdiff.cpp)
target_link_libraries(isisdiff ${isis_core_lib})
set_target_properties(isisdiff PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisdiff "get a \"diff\" of the metadata of two images")

add_executable(isisconv isisconv.cpp)
target_link_libraries(isisconv ${isis_core_lib})
set_target_properties(isisconv PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisconv "general MR image converter")

add_executable(isisflip isisflip.cpp)
target_link_libraries(isisflip ${isis_core_lib} ${isis_math_lib})
set_target_properties(isisflip PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisflip "flip MR images along specific dimensions")

add_executable(isisraw isisraw.cpp)
target_link_libraries(isisraw ${isis_core_lib})
set_target_properties(isisraw PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisraw "read or write raw data files from/to isis images")

# the install targets.
install(TARGETS
  isisdump
  isisdiff
  isisconv
  isisflip
  isisraw RUNTIME DESTINATION bin COMPONENT "CLI_Tools")

if(ISIS_QT5)
	find_package(Qt5Widgets)
	add_executable(isisview isisview.cpp)
	target_link_libraries(isisview ${isis_core_lib} ${isis_qt5_lib} Qt5::Widgets )
	set_target_properties(isisview PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
	build_manpage(isisview "simple mri data viewer")

	install(TARGETS isisview RUNTIME DESTINATION bin COMPONENT "CLI_Tools")
endif(ISIS_QT5)

