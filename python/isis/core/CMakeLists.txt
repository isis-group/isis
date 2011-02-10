
add_library(_core SHARED core.cpp _vector.hpp _application.hpp )
set_target_properties(_core PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES( _core ${ISIS_LIB_DEPENDS} ${ISIS_LIB} ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
SET(TARGETS ${TARETS} _core)

configure_file(${PROJECT_SOURCE_DIR}/isis/core/__init__.py ${PROJECT_BINARY_DIR}/isis/core/__init__.py COPYONLY)

INSTALL(FILES ${PROJECT_SOURCE_DIR}/isis/core/__init__.py DESTINATION ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR}/core)

INSTALL(TARGETS ${TARGETS}
     LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR}/core COMPONENT RuntimeLibraries)
