
IF(${CMAKE_PROJECT_NAME}_COREUTILS)
    add_library(python_coreutils coreutils.cpp )
    set_target_properties(python_coreutils PROPERTIES PREFIX "" OUTPUT_NAME "libisis-python")
    TARGET_LINK_LIBRARIES(python_coreutils utils ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
    SET(TARGETS ${TARETS} python_coreutils)
ENDIF(${CMAKE_PROJECT_NAME}_COREUTILS)

INSTALL(TARGETS ${TARGETS}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT RuntimeLibraries)