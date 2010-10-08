# the isis applications

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(ISIS_APPLICATIONS)

###########################################################
# find and configure the isis core library
###########################################################
SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH 
    "The directory where the isis cmake configuration skript script is located.")
FIND_PACKAGE(ISIS REQUIRED)
INCLUDE_DIRECTORIES(${ISIS_INCLUDE_DIRS})

# we use the log definitions of the core
IF(ISIS_RUNTIME_LOG)
	ADD_DEFINITIONS(-D_ENABLE_LOG=1)
ELSE(ISIS_RUNTIME_LOG)
	ADD_DEFINITIONS(-D_ENABLE_LOG=0)
ENDIF(ISIS_RUNTIME_LOG)

# enable DEBUG_LOG
IF(ISIS_DEBUG_LOG)
	ADD_DEFINITIONS(-D_ENABLE_DEBUG=1)
	MESSAGE(STATUS "Debug code enabled, this will will have a big impact on the performance")
ELSE(ISIS_DEBUG_LOG)
	ADD_DEFINITIONS(-D_ENABLE_DEBUG=0)
ENDIF(ISIS_DEBUG_LOG)

OPTION(${CMAKE_PROJECT_NAME}_REGISTRATION "Build the isis registration tool isisreg3d and isisdotrans3d" OFF)
OPTION(${CMAKE_PROJECT_NAME}_PROPERTYVIEWER "Build the graphical isis property viewer isispropertyviewer" OFF)


# the registration tools
if(${CMAKE_PROJECT_NAME}_REGISTRATION)   
    FIND_PACKAGE(ITK REQUIRED)
	INCLUDE(${USE_ITK_FILE})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-100")
	add_subdirectory(isisReg3D)
	add_subdirectory(isisDoTrans3D)
	set(PROGRAMS ${PROGRAMS} ${CMAKE_BINARY_DIR}/isisDoTrans3D/isisdotrans3d)
	set(PROGRAMS ${PROGRAMS} ${CMAKE_BINARY_DIR}/isisReg3D/isisreg3d)
endif(${CMAKE_PROJECT_NAME}_REGISTRATION)
	

INSTALL(PROGRAMS ${PROGRAMS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT runtime)

