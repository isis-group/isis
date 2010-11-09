
# add_executable(imageIOTest imageIOTest.cpp)
# target_link_libraries(imageIOTest utils data)

# add_executable(imageLoadNiiTest imageLoadNiiTest.cpp)
# target_link_libraries(imageLoadNiiTest utils data)

add_executable(imageIOLoadDicom imageIOLoadDicom.cpp)
add_executable(imageIONullTest imageIONullTest.cpp)
add_executable(imageIONiiTest imageIONiiTest.cpp)
add_executable(imageIOVistaTest imageIOVistaTest.cpp)
add_executable(imageIOTest imageIOTest.cpp)

target_link_libraries(imageIOLoadDicom ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
target_link_libraries(imageIONullTest  ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
target_link_libraries(imageIONiiTest   ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
target_link_libraries(imageIOVistaTest ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
target_link_libraries(imageIOTest      ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS})
