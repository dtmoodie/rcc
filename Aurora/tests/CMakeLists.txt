INCLUDE_DIRECTORIES("../")
add_executable(test_interface_hash test_interface_hash.cpp)
add_executable(test_shared_ptr test_shared_ptr.cpp)
target_link_libraries(test_shared_ptr RuntimeObjectSystem)
