find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_library(isisAdapter_qt4 SHARED qtapplication.cpp)
# use add_library with hpp files as well
target_link_libraries(isisAdapter_qt4 utils data ${QT_LIBRARIES})
