############################################################
# The ISIS project
# 
# CMake configuration file of the ISIS applications.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 16:30:42 +0200
# 
############################################################

include_directories(${CMAKE_SOURCE_DIR}/lib/core)

############################################################
# optional components
############################################################
option(ISIS_APPS_CALC "Build the ISIS image calculator" OFF)

# isiscalc
if(ISIS_APPS_CALC)
  set(APPS ${APPS} isiscalc)
  add_subdirectory(isiscalc)
endif(ISIS_APPS_CALC)

############################################################
# status message
############################################################
message(STATUS 
  "Installing the following applications to: ${CMAKE_INSTALL_PREFIX}/bin")
foreach(app ${APPS})
    message(STATUS " == ${app}")
endforeach(app ${APPS})
