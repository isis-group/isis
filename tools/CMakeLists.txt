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
include_directories(${CMAKE_SOURCE_DIR}/lib/Core)

set(MANPAGES OFF CACHE BOOL "Create manpages for the tools")

macro( build_manpage name)
	if(MANPAGES)
		add_custom_command(
			OUTPUT "${name}.1"
			DEPENDS ${name}
			COMMAND ${HELP2MAN} ./${name} --version-string=${ISIS_CORE_REV_STRING} --no-discard-stderr --no-info -o ${name}.1 -s 1
			VERBATIM
		)
		set(manpage_list ${manpage_list} "${name}.1")
	endif(MANPAGES)
endmacro( build_manpage )

if(MANPAGES)
	find_program(HELP2MAN help2man)
endif(MANPAGES)

############################################################
# configure the targets
############################################################
add_executable(isisdump isisdump.cpp)
target_link_libraries(isisdump isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isisdump PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisdump)

add_executable(isisdiff isisdiff.cpp)
target_link_libraries(isisdiff isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isisdiff PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisdiff)

add_executable(isisconv isisconv.cpp)
target_link_libraries(isisconv isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isisconv PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisconv)

add_executable(isisflip isisflip.cpp)
target_link_libraries(isisflip isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isisflip PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isisflip)

if(MANPAGES)
	add_custom_target(manpages ALL DEPENDS ${manpage_list})
endif(MANPAGES)

# the install targets.
install(TARGETS
  isisdump
  isisdiff
  isisconv
  isisflip
  RUNTIME DESTINATION bin COMPONENT "CLI_Tools")

