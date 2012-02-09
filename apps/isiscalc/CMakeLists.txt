############################################################
# The ISIS project
# 
# CMake configuration file of the ISIS application isiscalc.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 16:39:18 +0200
# 
############################################################

############################################################
# find dependencies
############################################################
find_library(LIB_MUPARSER "muparser" REQUIRED)
find_path(INCPATH_MUPARSER "muParser.h" PATH_SUFFIXES "muParser")
include_directories(${INCPATH_MUPARSER})
add_executable(isiscalc isiscalc.cpp)
target_link_libraries(isiscalc ${LIB_MUPARSER} isis_core ${ISIS_LIB_DEPENDS})
set_target_properties(isiscalc PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
build_manpage(isiscalc)

############################################################
# the install targets.
############################################################
install(TARGETS isiscalc RUNTIME DESTINATION bin COMPONENT "CLI Tools")

