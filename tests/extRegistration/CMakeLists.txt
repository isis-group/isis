
# look for itk package
FIND_PACKAGE(ITK)
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR
    "ITK not found. Please set ITK_DIR.")
ENDIF(ITK_FOUND)


add_executable(extRegistrationTest registrationFactory3DTest.cpp)
TARGET_LINK_LIBRARIES(extRegistrationTest ITKIO ITKSpatialObject ITKNumerics ITKStatistics)



