
add_executable(imageIOTest imageIOTest.cpp)
target_link_libraries(imageIOTest utils data)

add_executable(imageLoadNiiTest imageLoadNiiTest.cpp)
target_link_libraries(imageLoadNiiTest utils data)

add_executable(imageIOLoadDicom imageIOLoadDicom.cpp)
target_link_libraries(imageIOLoadDicom utils data)

add_executable(imageIONullTest imageIONullTest.cpp)
target_link_libraries(imageIONullTest utils data isisImageFormat_Null)
