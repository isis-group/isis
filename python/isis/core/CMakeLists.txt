############################################################
# The ISIS project
# 
# Sub configuration file of the ISIS core python extension
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 15:31:36 +0200
# 
############################################################

############################################################
# configure targets
############################################################
add_library(_core SHARED core.cpp _vector.hpp _application.hpp )
set_target_properties(_core PROPERTIES PREFIX "")
target_link_libraries( _core ${ISIS_LIB_DEPENDS} isis_core ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})

configure_file(__init__.py 
  ${CMAKE_BINARY_DIR}/python/isis/core/__init__.py COPYONLY)

############################################################
# configure installation
############################################################
install(FILES __init__.py 
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR}/core)

install(TARGETS _core
     LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR}/core
     COMPONENT RuntimeLibraries)
