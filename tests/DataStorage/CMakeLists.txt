
add_executable(chunkTest chunkTest.cpp)
add_executable(imageTest imageTest.cpp)
add_executable(imageListTest imageListTest.cpp)
add_executable(imageStresstest imageStresstest.cpp)

target_link_libraries(chunkTest utils data)
target_link_libraries(imageTest utils data)
target_link_libraries(imageListTest utils data)
target_link_libraries(imageStresstest utils data)
