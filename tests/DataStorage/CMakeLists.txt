
add_executable(chunkTest chunkTest.cpp)
add_executable(imageTest imageTest.cpp)

target_link_libraries(chunkTest utils data)
target_link_libraries(imageTest utils data)

