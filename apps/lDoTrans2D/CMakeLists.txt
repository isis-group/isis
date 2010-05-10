ADD_EXECUTABLE(ldotrans2d ldotrans2D.cxx)
TARGET_LINK_LIBRARIES(ldotrans2d  ITKIO ITKCommon extitk data utils itkAdapter viaio gslcblas)
