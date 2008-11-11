PROJECT(imageRegTool)


cmake_minimum_required(VERSION 2.6)


IF( BUILD_OUTSIDE_INSIGHT_APPLICATIONS )

  # The fluid-generated fltk sources have many warnings.  This macro
  # will disable warnings for the generated files on some compilers.
  MACRO ( ITK_DISABLE_FLTK_GENERATED_WARNINGS files )
    IF ( CMAKE_COMPILER_IS_GNUCXX )
      FOREACH ( f ${files} )
	STRING ( REGEX REPLACE "\\.fl$" ".cxx" SRC "${f}" )
	STRING ( REGEX REPLACE ".*/([^/]*)$" "\\1" SRC "${SRC}" )
	SET_SOURCE_FILES_PROPERTIES ( ${SRC} PROPERTIES COMPILE_FLAGS -w )
      ENDFOREACH ( f )
    ENDIF ( CMAKE_COMPILER_IS_GNUCXX )
  ENDMACRO ( ITK_DISABLE_FLTK_GENERATED_WARNINGS )

  FIND_PACKAGE(ITK)
  IF(ITK_FOUND)
    INCLUDE(${ITK_USE_FILE})
  ELSE(ITK_FOUND)
    MESSAGE(FATAL_ERROR
            "Cannot build InsightApplications without ITK.  Please set ITK_DIR.")
  ENDIF(ITK_FOUND)

  FIND_PACKAGE(FLTK)
  IF(FLTK_FOUND)
    INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIR})
  ENDIF(FLTK_FOUND)

ENDIF( BUILD_OUTSIDE_INSIGHT_APPLICATIONS )


IF(UNIX)
  INCLUDE( ${CMAKE_ROOT}/Modules/FindX11.cmake )
ENDIF(UNIX)

INCLUDE_DIRECTORIES(
  ${ITK_SOURCE_DIR}/Code/Common
  ${ITK_SOURCE_DIR}/Code/BasicFilters
  ${ITK_SOURCE_DIR}/Code/IO
  ${ITK_SOURCE_DIR}/Code/Numerics
  ${ITK_SOURCE_DIR}/Code/Numerics/vxl
  ${ITK_SOURCE_DIR}/Code/Numerics/Statistics
  ${ITK_SOURCE_DIR}/Utilities/MetaIO
  ${ITK_SOURCE_DIR}/Utilities/MetaIO/SaptialObject
  ${InsightApplications_SOURCE_DIR}/Auxiliary/FltkImageViewer
  ${InsightApplications_BINARY_DIR}/Auxiliary/FltkImageViewer
  ${InsightApplications_SOURCE_DIR}/Auxiliary/ITKFlFileWriter
  ${InsightApplications_SOURCE_DIR}/LandmarkInitializedMutualInformationRegistration
  ${InsightApplications_BINARY_DIR}/LandmarkInitializedMutualInformationRegistration
)

FLTK_WRAP_UI(imageRegTool guiMain.fl)
ITK_DISABLE_FLTK_GENERATED_WARNINGS(guiMain.fl)

ADD_EXECUTABLE(imageRegTool
    guiMainImplementation.cxx
    LandmarkRegistrator.cxx
    main.cxx
    ${imageRegTool_FLTK_UI_SRCS}
    )
INSTALL_TARGETS(/bin imageRegTool)

TARGET_LINK_LIBRARIES (imageRegTool
ITKIO
ITKMetaIO
ITKBasicFilters
ITKStatistics
ITKFlFileWriter
ITKFltkImageViewer
)

