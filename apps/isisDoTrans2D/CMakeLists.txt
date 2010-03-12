ADD_EXECUTABLE(isisdotrans2d isisdotrans2D.cxx)
TARGET_LINK_LIBRARIES(isisdotrans2d  ITKIO ITKCommon extitk viaio gslcblas)
