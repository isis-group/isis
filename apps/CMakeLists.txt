add_subdirectory(isisReg3D)
add_subdirectory(isisDoTrans3D)
add_subdirectory(isisCheckerboard3D)
#add_subdirectory(isisKaltmacher)

add_executable(isisdump isisdump.cpp)
target_link_libraries(isisdump data)
