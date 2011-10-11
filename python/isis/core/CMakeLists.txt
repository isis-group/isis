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
add_library(_core SHARED core.cpp _vector.hpp _application.hpp _property.hpp _propmap.hpp )
set_target_properties(_core PROPERTIES PREFIX "")
target_link_libraries( _core ${ISIS_LIB_DEPENDS} isis_core ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})

configure_file(__init__.py 
  ${CMAKE_BINARY_DIR}/python/isis/core/__init__.py COPYONLY)

############################################################
# configure installation
############################################################
install(FILES __init__.py 
  DESTINATION  ${PYTHON_INSTALL_PATH}/isis/core)

install(TARGETS _core
     LIBRARY DESTINATION  ${PYTHON_INSTALL_PATH}/isis/core
     COMPONENT RuntimeLibraries)
