include_directories(../lib/CoreUtils)

add_definitions(-D_ENABLE_FILEIO_LOG=1 -D_ENABLE_CORE_LOG=1)

add_executable(test_logger		test_logger.cpp)
add_executable(test_property	test_property.cpp)

target_link_libraries(test_logger utils)
target_link_libraries(test_property utils)
