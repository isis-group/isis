ADD_EXECUTABLE(isisreg3d isisreg3d.cxx)
TARGET_LINK_LIBRARIES(isisreg3d ITKIO ITKSpatialObject ITKNumerics ITKStatistics viaio  ${ISIS_LIB} ${ISIS_LIB_DEPENDS} )

