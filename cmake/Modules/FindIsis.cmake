# FindISIS
# 
# This script search the ISIS core library libsisis.so and the corresponding header files.
# It also looks for the ISIS build configuration file.
#
# Supports ISIS core version 0.2.0
#

# Provided variables:
#
# ISIS_FOUND             TRUE if all ISIS libraries were found, FALSE otherwise.
# ISIS_INCLUDE_DIR       the path to the header directory
# ISIS_SHARED_LIB        the isis shared library (e.g. libisis.so)
# ISIS_STATIC_LIB        the isis static linked library (e.g. libisis.a)
# ISIS_PLUGIN_DIR        the path where the IO plugins reside
#
# This script reads the configuration file isis_corecfg.cmake provided
# by the isis core package.
#

FIND_PATH(CFG_FILE isis_corecfg.cmake
  PATH ./
  DOC "The cmake configuration file containing the isis core library build settings.")

IF(${CFG_FILE})
  INCLUDE(${CFG_FILE})
ELSE(${CFG_FILE})
  MESSAGE(SEND_ERROR "Error, cannot find isis core configuration file.")
ENDIF(${CFG_FILE})
