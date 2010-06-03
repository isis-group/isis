add_executable(isisdump isisdump.cpp)
target_link_libraries(isisdump data)

add_executable(isisdiff isisdiff.cpp)
target_link_libraries(isisdiff data)

add_executable(isisconv isisconv.cpp)
target_link_libraries(isisconv data)
