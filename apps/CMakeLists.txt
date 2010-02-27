
IF(BUILD_APPLICATIONS_2D)
  add_subdirectory(isisReg2D)
  add_subdirectory(isisDoTrans2D)
ENDIF(BUILD_APPLICATIONS_2D)

IF(BUILD_APPLICATIONS_3D)
  add_subdirectory(isisReg3D)
  add_subdirectory(isisDoTrans3D)
ENDIF(BUILD_APPLICATIONS_3D)

add_subdirectory(isisCheckerboard3D)
#add_subdirectory(isisKaltmacher)
