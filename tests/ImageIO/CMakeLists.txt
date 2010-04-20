
# add_executable(imageIOTest imageIOTest.cpp)
# target_link_libraries(imageIOTest utils data)

# add_executable(imageLoadNiiTest imageLoadNiiTest.cpp)
# target_link_libraries(imageLoadNiiTest utils data)

if(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)
    add_executable(imageIOLoadDicom imageIOLoadDicom.cpp)
    target_link_libraries(imageIOLoadDicom utils data)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_DICOM)

if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)
    add_executable(imageIONullTest imageIONullTest.cpp)
    target_link_libraries(imageIONullTest utils data isisImageFormat_Null)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NULL)

if(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)
    add_executable(imageIONiiTest imageIONiiTest.cpp)
    target_link_libraries(imageIONiiTest utils data isisImageFormat_Nifti)
endif(${CMAKE_PROJECT_NAME}_IOPLUGIN_NIFTI)
