
FIND_PACKAGE(Boost COMPONENTS unit_test_framework filesystem regex system)
include_directories(${Boost_INCLUDE_DIR}  lib)

IF(BUILD_COREUTILS)
	add_subdirectory(CoreUtils)
ENDIF(BUILD_COREUTILS)

IF(BUILD_DATA_IO)
	add_subdirectory(DataStorage)
	add_subdirectory(ImageIO)
ENDIF(BUILD_DATA_IO)

IF(BUILD_ITK)
	add_subdirectory(extRegistration)
ENDIF(BUILD_ITK)
