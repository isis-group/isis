# the registration component, part of the ISIS library

# finde the ITK libraries
find_package(ITK)
if(ITK_FOUND)
    include(${ITK_USE_FILE})
else(ITK_FOUND)
    message(FATAL_ERROR "ITK not found. Please set ITK_DIR")
endif(ITK_FOUND)  

add_library(isisreg SHARED  isisRegistrationFactory3D.cxx)

set_target_properties(isisreg PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LINK_FLAGS "--no-undefined")
    
target_link_libraries(isisreg ITKNumerics)
