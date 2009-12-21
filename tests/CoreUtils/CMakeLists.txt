
add_executable(propertyTest propertyTest.cpp)
add_executable(typeTest typeTest.cpp)
add_executable(typePtrTest typePtrTest.cpp)
add_executable(vectorTest vectorTest.cpp)
add_executable(propMapTest propMapTest.cpp)

target_link_libraries(propertyTest utils)
target_link_libraries(typeTest utils)
target_link_libraries(typePtrTest utils)
target_link_libraries(vectorTest utils)
target_link_libraries(propMapTest utils)
