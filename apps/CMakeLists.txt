OPTION(BUILD_APPLICATIONS_2D "Support for 2d images" ON)
OPTION(BUILD_APPLICATIONS_3D "Support for 3d images" ON)

IF(BUILD_APPLICATIONS_2D)
	add_subdirectory(isisReg2D)
	add_subdirectory(isisDoTrans2D)
ENDIF(BUILD_APPLICATIONS_2D)

IF(BUILD_APPLICATIONS_3D)
	add_subdirectory(isisReg3D)
	add_subdirectory(isisDoTrans3D)
	add_subdirectory(isisCheckerboard3D)
	#add_subdirectory(isisDoTrans3D_float)
ENDIF(BUILD_APPLICATIONS_3D)

#add_subdirectory(isisDoTrans3D_float)
#add_subdirectory(isisKaltmacher)
