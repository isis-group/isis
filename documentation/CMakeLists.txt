cmake_minimum_required(VERSION 2.6)

###############################################################################
# the documentation subdirectory
###############################################################################

OPTION(${CMAKE_PROJECT_NAME}_SPHINX_DOCUMENTATION "Build the sphinx-based documentation")

IF (${CMAKE_PROJECT_NAME}_SPHINX_DOCUMENTATION)
	FIND_FILE(SPHINX sphinx-build
	PATHS /usr/bin /usr/local/bin
	DOC "Python sphinx to create documentation")
	
	IF (NOT SPHINX)
		MESSAGE(FATAL ERROR "sphinx could not be found on your system. Please install it to build the documentation")
	ENDIF (NOT SPHINX)
	
	ADD_CUSTOM_TARGET(sphinx-documentation ALL "documentation/apply_sphinx.sh" 
		${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} 
		"${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH}"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
ENDIF (${CMAKE_PROJECT_NAME}_SPHINX_DOCUMENTATION)
