# the isis python libraries

OPTION(${CMAKE_PROJECT_NAME}_PYTHON_COREUTILS "Build coreutils python library" ON)
OPTION(${CMAKE_PROJECT_NAME}_PYTHON_DATASTORAGE "Build datastorage python library" ON)
OPTION(${CMAKE_PROJECT_NAME}_PYTHON_IMAGEIO "Build imageio python library" ON)

FIND_PACKAGE(Boost REQUIRED COMPONENTS python)
FIND_PACKAGE(PythonLibs REQUIRED) 
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

add_subdirectory(isis)
configure_file(${PROJECT_SOURCE_DIR}/python/isis/__init__.py ${PROJECT_BINARY_DIR}/python/isis/__init__.py COPYONLY)



