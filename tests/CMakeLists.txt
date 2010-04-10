
FIND_PACKAGE(Boost REQUIRED COMPONENTS unit_test_framework filesystem regex system)
include_directories(${Boost_INCLUDE_DIR}  lib)
include_directories(${ITK_INCLUDE_DIRS})

IF(BUILD_COREUTILS)
	add_subdirectory(CoreUtils)
ENDIF(BUILD_COREUTILS)

IF(BUILD_DATA_IO)
	add_subdirectory(DataStorage)
	add_subdirectory(ImageIO)
ENDIF(BUILD_DATA_IO)

IF(HAVE_ITK)
	add_subdirectory(extRegistration)
ENDIF(HAVE_ITK)

IF(HAVE_VTK OR HAVE_ITK)
    add_subdirectory(ExternalLibraryAdapter)
ENDIF(HAVE_VTK OR HAVE_ITK)

add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger utils ${Boost_LIBRARIES})

add_executable(test_parameter test_parameter.cpp)
target_link_libraries(test_parameter utils ${Boost_LIBRARIES})
