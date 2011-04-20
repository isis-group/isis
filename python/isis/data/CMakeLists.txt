############################################################
# The ISIS project
# 
# Sub configuration file of the ISIS datastorage python extension
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 15:49:23 +0200
# 
############################################################

include_directories(${CMAKE_SOURCE_DIR}/python/isis)

############################################################
# configure targets
############################################################
add_library(_data SHARED data.cpp _image.hpp )
set_target_properties(_data PROPERTIES PREFIX "")
target_link_libraries( _data ${ISIS_LIB_DEPENDS} isis_core ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})

configure_file(__init__.py 
  ${CMAKE_BINARY_DIR}/python/isis/data/__init__.py COPYONLY)

############################################################
# configure installation
############################################################
install(FILES __init__.py 
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR}/data)

install(TARGETS _data
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR}/data 
  COMPONENT RuntimeLibraries)
