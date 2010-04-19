cmake_minimum_required(VERSION 2.6)

###############################################################################
# the design subdirectory
###############################################################################

OPTION(${CMAKE_PROJECT_NAME}_USE_ASTYLE 
    "Create a make target for the Artistic Style code formatter" OFF)

IF (${CMAKE_PROJECT_NAME}_USE_ASTYLE)
  FIND_FILE(ASTYLE astyle
  PATHS /usr/bin /usr/local/bin
  DOC "The Artistic Style code formatter.")

  IF(NOT ASTYLE)
    MESSAGE(FATAL ERROR"astyle executable not found. Please enter the full path to the Artistic Style code formatter tool.")
  ENDIF(NOT ASTYLE)

  MESSAGE(STATUS "Using Artistic Style: ${ASTYLE}")
  EXECUTE_PROCESS(COMMAND ${ASTYLE} "--version"
    ERROR_VARIABLE ASTYLE_VERSION
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  MESSAGE(STATUS ${ASTYLE_VERSION})

  # create a target
  ADD_CUSTOM_TARGET(astyle 
      "design/apply_astyle.sh" ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

ENDIF (${CMAKE_PROJECT_NAME}_USE_ASTYLE)


