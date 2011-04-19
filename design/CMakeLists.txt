############################################################
# The ISIS project
# 
# CMake configuration file of the ISIS code style tools
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Fri, 08 Apr 2011 11:35:35 +0200
# 
############################################################

############################################################
# optional components
############################################################
option(ISIS_USE_ASTYLE 
    "Create a make target for the Artistic Style code formatter" OFF)

############################################################
# create astyle target
############################################################
if (ISIS_USE_ASTYLE)
  
  # find astyle and configuration options
  find_file(ASTYLE astyle
    PATHS /usr/bin /usr/local/bin
    DOC "The Artistic Style code formatter.")

  if(NOT ASTYLE)
    message(FATAL ERROR 
      "astyle executable not found. Please enter the full path to the Artistic Style code formatter tool.")
  endif(NOT ASTYLE)

  message(STATUS "Using Artistic Style: ${ASTYLE}")
  execute_process(COMMAND ${ASTYLE} "--version"
    ERROR_VARIABLE ASTYLE_VERSION
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  message(STATUS ${ASTYLE_VERSION})

  # create a target
  add_custom_target(astyle 
      "design/apply_astyle.sh" ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

endif (ISIS_USE_ASTYLE)
