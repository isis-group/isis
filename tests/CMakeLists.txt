
IF(BUILD_COREUTILS)
add_executable(test_logger		test_logger.cpp)
add_executable(test_property	test_property.cpp)
add_subdirectory(CoreUtils)

target_link_libraries(test_logger 	utils)
target_link_libraries(test_property utils)
ENDIF(BUILD_COREUTILS)

IF(BUILD_DATA_IO)
# add_executable(test_data		test_data.cpp)
add_subdirectory(DataStorage)
# target_link_libraries(test_data    	utils data)
ENDIF(BUILD_DATA_IO)
#add_executable(test_imageio	test_imageio.cpp)

IF(BUILD_ITK)
add_subdirectory(extRegistration)
ENDIF(BUILD_ITK)

#target_link_libraries(test_imageio 	utils data)

