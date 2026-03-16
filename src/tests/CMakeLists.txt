add_executable(test_alias_method test_alias_method.cpp)
target_link_libraries(test_alias_method PRIVATE luisa::render)

add_executable(test_u64 test_u64.cpp)
target_link_libraries(test_u64 PRIVATE luisa::render)
# test_u64: pure CPU arithmetic; fast and deterministic – register with CTest.
add_test(NAME test_u64 COMMAND test_u64)
set_tests_properties(test_u64 PROPERTIES TIMEOUT 60)

# sky texture precomputation test
add_executable(test_sky test_sky.cpp)
target_link_libraries(test_sky PRIVATE luisa-render-texture-sky-precompute)
# test_sky: runs CPU precomputation (no GPU backend required) but writes a file;
# register with CTest and cap it at 120 s to keep CI sane.
add_test(NAME test_sky COMMAND test_sky)
set_tests_properties(test_sky PROPERTIES TIMEOUT 120)

add_executable(test_sphere test_sphere.cpp)
target_link_libraries(test_sphere PRIVATE luisa::render)
# test_sphere: runs CPU subdivision loop (no GPU required); writes OBJ files.
add_test(NAME test_sphere COMMAND test_sphere)
set_tests_properties(test_sphere PROPERTIES TIMEOUT 120)

# test_alias_method: runs 1 B sampling iterations – accurate benchmark but
# intentionally excluded from regular CTest to keep CI wall-time low.
# Run manually with: ./test_alias_method
