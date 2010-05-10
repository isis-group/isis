ADD_EXECUTABLE(ldotrans3d ldotrans3D.cxx)
TARGET_LINK_LIBRARIES(ldotrans3d  ITKIO ITKCommon extitk data utils itkAdapter viaio gslcblas)
