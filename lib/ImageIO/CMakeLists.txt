if(IOPLUGIN_NIFTI)
	find_library(LIB_NIFTIIO niftiio)
	find_library(LIB_ZNZ znz)
	find_path(INCPATH_NIFTI "nifti1_io.h" PATH_SUFFIXES "nifti")
	find_path(INCPATH_ZNZ "znzlib.h" PATH_SUFFIXES "nifti")

	include_directories(${INCPATH_NIFTI} ${INCPATH_ZNZ})
	add_library(isisImageFormat_Nifti SHARED imageFormat_Nifti.cpp)
	target_link_libraries(isisImageFormat_Nifti utils data ${LIB_NIFTIIO} ${LIB_ZNZ})

	set_target_properties(isisImageFormat_Nifti PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
		LINK_FLAGS "--no-undefined")
	set_target_properties(isisImageFormat_Nifti PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif(IOPLUGIN_NIFTI)


if(IOPLUGIN_NULL)
	add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)
	target_link_libraries(isisImageFormat_Null utils data)

	set_target_properties(isisImageFormat_Null PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
		LINK_FLAGS "--no-undefined")

	set_target_properties(isisImageFormat_Null PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif(IOPLUGIN_NULL)

if(IOPLUGIN_DICOM)
	add_library(isisImageFormat_Dicom SHARED imageFormat_Dicom.cpp)
	target_link_libraries(isisImageFormat_Dicom utils data dcmdata dcmimgle ofstd z)
	
	set_target_properties(isisImageFormat_Dicom PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	LINK_FLAGS "--no-undefined")
	set_target_properties(isisImageFormat_Dicom PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif(IOPLUGIN_DICOM)
	
message(STATUS "Installing plugins to ${CMAKE_BINARY_DIR}")

