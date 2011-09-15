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

execute_process ( COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(__init__.py
  ${CMAKE_BINARY_DIR}/python/isis/__init__.py COPYONLY)

############################################################
# installation
############################################################
message(STATUS 
  "Installing python modules to ${PYTHON_SITE_PACKAGES}") 
install(FILES __init__.py
  DESTINATION ${PYTHON_SITE_PACKAGES})


add_subdirectory(core)
add_subdirectory(data)
