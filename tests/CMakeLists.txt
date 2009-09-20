
add_executable(test_logger		test_logger.cpp)
add_executable(test_property	test_property.cpp)
add_executable(test_data		test_data.cpp)
#add_executable(test_imageio	test_imageio.cpp)

target_link_libraries(test_logger 	utils)
target_link_libraries(test_property utils)
target_link_libraries(test_data    	utils data)
#target_link_libraries(test_imageio 	utils data)
