#list all source files here
# add_library(isisImageFormatDCM SHARED ImageFormatDCM.cpp)
# add_library(isisImageFormatNii SHARED ImageFormatNii.cpp)
# 
# set_target_properties(isisImageFormatDCM PROPERTIES 
#     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#     LINK_FLAGS "--no-undefined")
#     
# set_target_properties(isisImageFormatNii PROPERTIES 
#     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#     LINK_FLAGS "--no-undefined")

add_library(isisImageFormat_Null SHARED imageFormat_Null.cpp)
