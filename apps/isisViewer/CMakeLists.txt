############################################################
# The ISIS project
#
# Main CMake configuration file of the ISIS project.
#
# Author: Thomas Proeger <thomasproeger@googlemail.com>
# Date: Wed, 04 May 2011 16:55:12 +0200
#
############################################################

# collecting files
file (GLOB VIEWER_FILES_HPP viewer/*.h viewer/*.hpp)
file (GLOB VIEWER_FILES_CPP viewer/*.c viewer/*.cpp)
file (GLOB GLWIDGET_FILES_HPP GLWidget/*.hpp GLWidget/*.h)
file (GLOB GLWIDGET_FILES_CPP GLWidget/*.cpp GLWidget/*.c)
file (GLOB UI_FILES forms/*.ui)
file (GLOB RESOURCE_FILES resources/*.qrc)

# extended search space for header 
include_directories(
  ${CMAKE_SOURCE_DIR}/lib 	#find qt4 adapter header 
  ${CMAKE_CURRENT_BINARY_DIR})	#find moc generated header files

###########################################################
# qt4 stuff
###########################################################

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
include(${QT_USE_FILE})

set(QT_USE_QTUITOOLS TRUE)
set(QT_USE_QTOPENGL TRUE)
set( QT_USE_OPENGL TRUE )

qt4_add_resources(isisViewer_rcc_files ${RESOURCE_FILES})

qt4_wrap_cpp(isisViewer_moc_files ${VIEWER_FILES_HPP} ${GLWIDGET_FILES_HPP})
qt4_wrap_ui(isisViewer_UIS_H ${UI_FILES})

###########################################################
# opengl stuff
###########################################################

find_package(OpenGL REQUIRED)

message(STATUS "openGL library: ${OPENGL_LIBRARY}")

include_directories(${OPENGL_INCLUDE_DIR})

###########################################################
# boost and common stuff
###########################################################

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/GLWidget)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/viewer)

#this flags has to be set due to use parallel with boos::signal
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_KEYWORDS -ftemplate-depth-100 -Wextra -Wshadow  -Wfatal-errors" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DQT_NO_KEYWORDS -Wextra -Wshadow  -Wfatal-errors " )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-100")

add_executable(isisViewer main.cpp ${GLWIDGET_FILES_CPP} ${VIEWER_FILES_CPP} ${isisViewer_UIS_H} ${isisViewer_moc_files} ${isisViewer_rcc_files})
find_library(isisqt4_LIBRARY "isisAdapter_qt4")

set(NEEDED_LIBS
	${OPENGL_LIBRARY} 
	${QT_LIBRARIES}
	${Boost_LIBRARIES}
	${ISIS_LIB}
	${ISIS_LIB_DEPENDS}
	isisAdapter_qt4 )

target_link_libraries(isisViewer ${NEEDED_LIBS})

