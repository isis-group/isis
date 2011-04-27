############################################################
# The ISIS project
# 
# Sub configuration file of the ISIS python extension
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Thu, 07 Apr 2011 15:25:04 +0200
# 
############################################################

############################################################
# configure python module
############################################################
set(ISIS_PYTHON_INSTALL_DIR 
  "lib/python${CURRENT_PYTHON_VERSION}/site-packages/isis")

configure_file(__init__.py
  ${CMAKE_BINARY_DIR}/python/isis/__init__.py COPYONLY)

############################################################
# installation
############################################################
message(STATUS 
  "Installing python modules to ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR}") 
install(FILES __init__.py
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${ISIS_PYTHON_INSTALL_DIR})

add_subdirectory(core)
add_subdirectory(data)
