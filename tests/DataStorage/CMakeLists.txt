
add_executable( chunkTest chunkTest.cpp )
add_executable( sortedchunklistTest sortedchunklistTest.cpp)
add_executable( imageTest imageTest.cpp )
add_executable( imageListTest imageListTest.cpp )
add_executable( imageStresstest imageStresstest.cpp )
add_executable( typePtrTest typePtrTest.cpp )

target_link_libraries( typePtrTest ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} )
target_link_libraries( chunkTest ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} )
target_link_libraries( sortedchunklistTest ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} )
target_link_libraries( imageTest ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} )
target_link_libraries( imageListTest ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} )
target_link_libraries( imageStresstest ${Boost_LIBRARIES} ${ISIS_LIB} ${ISIS_LIB_DEPENDS} )

