# local include directory
INCLUDE_DIRECTORIES(.)

if(${CMAKE_PROJECT_NAME}_CORE)
    add_subdirectory(core)
endif(${CMAKE_PROJECT_NAME}_CORE)

if(${CMAKE_PROJECT_NAME}_DATASTORAGE)
    add_subdirectory(data)
endif(${CMAKE_PROJECT_NAME}_DATASTORAGE)
