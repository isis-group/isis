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
add_library(_core SHARED core.cpp _vector.hpp _application.cpp _propmap.cpp _convertFromPython.cpp _convertToPython.hpp )
set_target_properties(_core PROPERTIES PREFIX "")
target_link_libraries( _core ${ISIS_LIB_DEPENDS} isis_core ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})

configure_file(__init__.py 
  ${CMAKE_BINARY_DIR}/python/isis/core/__init__.py COPYONLY)

############################################################
# configure installation
############################################################
foreach(pv ${PYTHON_VERSIONS})
execute_process ( COMMAND ${pv} -c "from distutils.sysconfig import get_python_lib; print get_python_lib().lstrip('${PYTHON_PACKAGE_PATH_PREFIX}')" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
set(PYTHON_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/${PYTHON_SITE_PACKAGES}")

install(FILES __init__.py 
  DESTINATION  ${PYTHON_INSTALL_PATH}/isis/core)

install(TARGETS _core
     LIBRARY DESTINATION  ${PYTHON_INSTALL_PATH}/isis/core
     COMPONENT RuntimeLibraries)
endforeach(pv)