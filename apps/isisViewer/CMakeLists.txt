cmake_minimum_required(VERSION 2.6)

# collecting files
file (GLOB HEADER_FILES headers/*.h headers/*.hpp)
file (GLOB CPP_FILES sources/*.cpp *.cpp)
file (GLOB UI_FILES forms/*.ui)
file (GLOB HELPER_FILES helper/*.cpp)

###########################################################
# isis stuff
###########################################################
SET(ISIS_DIR /usr/share/isis/cmake CACHE PATH 
    "The directory where the isis cmake configuration skript script is located.")
find_package(ISIS REQUIRED)
include_directories(${ISIS_INCLUDE_DIRS})

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

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${ISIS_INCLUDE_DIRS})

###########################################################
# qt4 stuff
###########################################################

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
INCLUDE(${QT_USE_FILE})

set(QT_USE_QTUITOOLS TRUE)
set(QT_USE_QTOPENGL TRUE)
set( QT_USE_OPENGL TRUE )


qt4_wrap_cpp(isisViewer_moc_files ${HEADER_FILES})
QT4_WRAP_UI(isisViewer_UIS_H ${UI_FILES})

###########################################################
# opengl stuff
###########################################################

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

message(STATUS "openGL library: ${OPENGL_LIBRARY}")

include_directories(${OPENGL_INCLUDE_DIR})

###########################################################
# boost and common stuff
###########################################################

FIND_PACKAGE(Boost REQUIRED COMPONENTS signals)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/helper)

#this flags has to be set due to use parallel with boos::signal
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_KEYWORDS -ftemplate-depth-100 -Wextra -Wshadow  -Wfatal-errors" )
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DQT_NO_KEYWORDS -Wextra -Wshadow  -Wfatal-errors " )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-100")

add_executable(isisViewer ${CPP_FILES} ${HELPER_FILES} ${isisViewer_UIS_H} ${isisViewer_moc_files})

set(NEEDED_LIBS
	${OPENGL_LIBRARY} 
	${QT_LIBRARIES}
	${Boost_LIBRARIES}
	${ISIS_LIB}
	${ISIS_LIB_DEPENDS}
	${CMAKE_INSTALL_PREFIX}/lib/libisisAdapter_qt4.so )

target_link_libraries(isisViewer ${NEEDED_LIBS})

