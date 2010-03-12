ADD_EXECUTABLE(isisdotrans3d isisdotrans3D.cxx)
TARGET_LINK_LIBRARIES(isisdotrans3d  ITKIO ITKCommon extitk viaio gslcblas)
