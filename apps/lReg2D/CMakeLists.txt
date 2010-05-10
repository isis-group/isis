ADD_EXECUTABLE(lreg2d lreg2D.cxx)
TARGET_LINK_LIBRARIES(lreg2d  ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio utils data itkAdapter gslcblas)
