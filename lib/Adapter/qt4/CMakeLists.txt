find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_library(isisAdapter_qt4 SHARED qtapplication.cpp)
# use add_library with hpp files as well
target_link_libraries(isisAdapter_qt4 ${ISIS_LIB} ${QT_LIBRARIES})
SET(TARGETS ${TARGETS} isisAdapter_qt4)

INSTALL(TARGETS ${TARGETS}
  LIBRARY DESTINATION  ${CMAKE_INSTALL_PREFIX}/lib COMPONENT RuntimeLibraries)