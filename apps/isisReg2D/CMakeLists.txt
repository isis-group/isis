ADD_EXECUTABLE(isisreg2d isisreg2D.cxx)
TARGET_LINK_LIBRARIES(isisreg2d  ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio utils data itkAdapter gslcblas)
