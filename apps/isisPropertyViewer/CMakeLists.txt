# collecting files
file (GLOB HEADER_FILES headers/*.h headers/*.hpp)
file (GLOB CPP_FILES sources/*.cpp *.cpp)
file (GLOB UI_FILES forms/*.ui)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)

#setup qt
set(QT_USE_QTUITOOLS TRUE)

qt4_wrap_cpp(isisPropertyViewer_moc_files ${HEADER_FILES})

QT4_WRAP_UI(isisPropertyViewer_UIS_H ${UI_FILES})

add_executable(isisPropertyViewer ${CPP_FILES} ${isisPropertyViewer_UIS_H} ${isisPropertyViewer_moc_files})

target_link_libraries(isisPropertyViewer ${QT_LIBRARIES} data util)




