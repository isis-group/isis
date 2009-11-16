
IF(BUILD_COREUTILS)
	add_subdirectory(CoreUtils)
ENDIF(BUILD_COREUTILS)

IF(BUILD_DATA_IO)
	add_subdirectory(DataStorage)
ENDIF(BUILD_DATA_IO)

IF(BUILD_ITK)
	add_subdirectory(extRegistration)
ENDIF(BUILD_ITK)


