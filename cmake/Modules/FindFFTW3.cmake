# - FindFFTW3.cmake
#
# Author: Thomas Proeger
#
# This cmake find module looks for the header and library files from the
# libfftw3-dev package.
#
# The following variables are exported:
#
# FFTW3_INCLUDE_DIR    - the directory containing the header files
# FFTW3_FFTW3_LIBRARY  - the libfftw3.so library file
# FFTW3_FFTW3L_LIBRARY - the libfftw3l.so library file
# FFTW3_FFTW3f_LIBRARY - the libfftw3f.so library file
# FFTW3_FOUND          - TRUE if and only if ALL other variables have correct
#                        values.
#

# the header files
FIND_PATH(FFTW3_INCLUDE_DIR
	NAMES fftw3.h
	DOC "The path to the fftw3 header file"
	)

# the library files
FIND_LIBRARY(FFTW3_FFTW3_LIBRARY
	NAMES fftw3
	DOC "The library file libfftw3"
	)

FIND_LIBRARY(FFTW3_FFTW3L_LIBRARY
	NAMES fftw3l
	DOC "The library file libfftw3l"
	)

FIND_LIBRARY(FFTW3_FFTW3F_LIBRARY
	NAMES fftw3f
	DOC "The library file libfftw3f"
	)


# handle the QUIETLY and REQUIRED arguments and set PNG_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTW3
	"Cannot find package FFTW3. Did you install 'libfftw3-dev'?"
  FFTW3_INCLUDE_DIR
	FFTW3_FFTW3_LIBRARY
#	FFTW3_FFTW3L_LIBRARY
#	FFTW3_FFTW3F_LIBRARY
	)

# these variables are only visible in 'advanced mode'
MARK_AS_ADVANCED(FFTW3_INCLUDE_DIR
	FFTW3_FFTW3_LIBRARY
	FFTW3_FFTW3L_LIBRARY
	FFTW3_FFTW3F_LIBRARY
	)

