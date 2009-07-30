include_directories(../../lib/EventLogger)

add_definitions(-D_ENABLE_FILEIO_LOG=1)
add_executable(eventloggertest example.cpp)

target_link_libraries(eventloggertest logger)

