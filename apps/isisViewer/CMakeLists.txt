cmake_minimum_required(VERSION 2.6)

# collecting files
file (GLOB HEADER_FILES headers/*.h headers/*.hpp)
file (GLOB CPP_FILES sources/*.cpp *.cpp)
file (GLOB UI_FILES forms/*.ui)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${ISIS_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)

FIND_PACKAGE(Boost REQUIRED COMPONENTS signals)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
#setup 
set(QT_USE_QTUITOOLS TRUE)

set(VTK_USE_QVTK TRUE)
set(VTK_USE_GUISUPPORT TRUE)
#this flags has to be set due to use parallel with boos::signal
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_KEYWORDS" )
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DQT_NO_KEYWORDS" )

qt4_wrap_cpp(isisViewer_moc_files ${HEADER_FILES})

QT4_WRAP_UI(isisViewer_UIS_H ${UI_FILES})

add_executable(isisViewer ${CPP_FILES} ${isisViewer_UIS_H} ${isisViewer_moc_files})

target_link_libraries(isisViewer ${QT_LIBRARIES} QVTK ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${CMAKE_INSTALL_PREFIX}/lib/libisisAdapter_vtk.so ${CMAKE_INSTALL_PREFIX}/lib/libisisAdapter_qt4.so)




