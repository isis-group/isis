include_directories(../../lib/EventLogger)
include_directories(../../lib/DataStorage)

add_definitions(-D_ENABLE_FILEIO_LOG=1)
add_executable(propertytest propertytest.cpp)

target_link_libraries(propertytest logger property)

