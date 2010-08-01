
IF(${CMAKE_PROJECT_NAME}_PYTHON_COREUTILS)
    add_library(coreutils coreutils.cpp )
    set_target_properties(coreutils PROPERTIES PREFIX "")
    TARGET_LINK_LIBRARIES(coreutils utils ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
ENDIF(${CMAKE_PROJECT_NAME}_PYTHON_COREUTILS)


