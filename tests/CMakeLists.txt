cmake_minimum_required(VERSION 2.4)

add_executable(test_logger		test_logger.cpp)
add_executable(test_property	test_property.cpp)
add_executable(test_data		test_data.cpp)

target_link_libraries(test_logger utils)
target_link_libraries(test_property utils)
target_link_libraries(test_data utils data)
