if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)
	find_library(LIB_NIFTIIO niftiio)
	find_library(LIB_ZNZ znz)
	find_path(INCPATH_NIFTI "nifti1_io.h" PATH_SUFFIXES "nifti")
	find_path(INCPATH_ZNZ "znzlib.h" PATH_SUFFIXES "nifti")

	include_directories(${INCPATH_NIFTI} ${INCPATH_ZNZ})
	add_library(isisImageFormat_Nifti SHARED imageFormat_Nifti.cpp)
	target_link_libraries(isisImageFormat_Nifti ${LIB_NIFTIIO} ${LIB_ZNZ} ${ISIS_DATA_LIBS})

	set_target_properties(isisImageFormat_Nifti PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_PATH})
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)


if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)
	add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)
	target_link_libraries(isisImageFormat_Null ${ISIS_DATA_LIBS})

	set_target_properties(isisImageFormat_Null PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_PATH})
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)

if(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ)
	FIND_LIBRARY(LIB_Z "z")
	FIND_PATH(INCPATH_GZIP "zlib.h")
	INCLUDE_DIRECTORIES(${GZIP_INC})

	add_library(isisImageFormat_gz_proxy SHARED isisImageFormat_gz_proxy.cpp)
	target_link_libraries(isisImageFormat_gz_proxy ${LIB_Z} ${ISIS_DATA_LIBS})

	set_target_properties(isisImageFormat_gz_proxy PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_PATH})
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_GZ)


if(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)
	find_library(LIB_DCMIMGLE dcmimgle)
	find_library(LIB_DCMIMAGE dcmimage)
	find_library(LIB_DCMDATA dcmdata)
	find_library(LIB_OFSTD ofstd)
	find_library(LIB_Z z)
	find_library(LIB_TIFF tiff)
	find_library(LIB_PNG png12)

	find_path(INCPATH_DCMTK "dcmtk/dcmdata/dcfilefo.h")
	include_directories(${INCPATH_DCMTK})

	add_library(isisImageFormat_Dicom SHARED imageFormat_Dicom.cpp imageFormat_DicomParser.cpp)
	target_link_libraries(isisImageFormat_Dicom ${LIB_DCMDATA} ${LIB_DCMIMGLE} ${LIB_OFSTD} ${LIB_Z} ${ISIS_DATA_LIBS} ${LIB_DCMIMAGE} ${LIB_TIFF} ${LIB_PNG})

	set_target_properties(isisImageFormat_Dicom PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_PATH})
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)


message(STATUS "Installing plugins to ${PLUGIN_PATH}")

