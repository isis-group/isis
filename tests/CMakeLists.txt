include_directories(../lib/EventLogger)
include_directories(../lib/DataStorage)

add_definitions(-D_ENABLE_FILEIO_LOG=1)

add_executable(test_logger		test_logger.cpp)
add_executable(test_property	test_property.cpp)

target_link_libraries(test_logger logger)
target_link_libraries(test_property  property)
