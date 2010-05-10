ADD_EXECUTABLE(lreg3d lreg3D.cxx)
TARGET_LINK_LIBRARIES(lreg3d  ITKIO ITKSpatialObject ITKNumerics ITKStatistics extitk viaio data itkAdapter utils gslcblas)
