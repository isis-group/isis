ADD_EXECUTABLE(isisreg3d isisreg3D.cxx)
TARGET_LINK_LIBRARIES(isisreg3d  ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio utils gslcblas)
