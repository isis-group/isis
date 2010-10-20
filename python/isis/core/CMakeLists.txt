
add_library(_core SHARED core.cpp _vector.hpp _application.hpp )
set_target_properties(_core PROPERTIES PREFIX "")
TARGET_LINK_LIBRARIES( _core ${ISIS_LIB_DEPENDS} ${ISIS_LIB} ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
SET(TARGETS ${TARETS} _core)

#INSTALL(TARGETS ${TARGETS}
 #     LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT RuntimeLibraries)