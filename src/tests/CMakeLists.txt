add_executable(test_alias_method test_alias_method.cpp)
target_link_libraries(test_alias_method PRIVATE luisa::render)

# ─── Unit tests ──────────────────────────────────────────────────────────────
# These are fast, deterministic, and require no GPU backend.

add_executable(test_u64 test_u64.cpp)
target_link_libraries(test_u64 PRIVATE luisa::render)
# test_u64: pure CPU 64-bit arithmetic – register with CTest.
add_test(NAME test_u64 COMMAND test_u64)
set_tests_properties(test_u64 PROPERTIES TIMEOUT 60 LABELS unit)

add_executable(test_complex test_complex.cpp)
target_link_libraries(test_complex PRIVATE luisa::render)
# test_complex: unit tests for Complex<T> arithmetic (util/complex.h).
add_test(NAME test_complex COMMAND test_complex)
set_tests_properties(test_complex PROPERTIES TIMEOUT 30 LABELS unit)

add_executable(test_colorspace test_colorspace.cpp)
target_link_libraries(test_colorspace PRIVATE luisa::render)
# test_colorspace: unit tests for XYZ<->sRGB<->Lab roundtrips (util/colorspace.h).
add_test(NAME test_colorspace COMMAND test_colorspace)
set_tests_properties(test_colorspace PROPERTIES TIMEOUT 30 LABELS unit)

add_executable(test_alias_table_unit test_alias_table_unit.cpp)
target_link_libraries(test_alias_table_unit PRIVATE luisa::render)
# test_alias_table_unit: deterministic unit tests for create_alias_table().
add_test(NAME test_alias_table_unit COMMAND test_alias_table_unit)
set_tests_properties(test_alias_table_unit PROPERTIES TIMEOUT 30 LABELS unit)

# ─── Integration tests ───────────────────────────────────────────────────────
# Exercise multiple components together (CPU only, no GPU backend needed).

# sky texture precomputation test
add_executable(test_sky test_sky.cpp)
target_link_libraries(test_sky PRIVATE luisa-render-texture-sky-precompute)
# test_sky: runs CPU Nishita sky precomputation; writes an EXR output file.
add_test(NAME test_sky COMMAND test_sky)
set_tests_properties(test_sky PROPERTIES TIMEOUT 120 LABELS integration)

add_executable(test_sphere test_sphere.cpp)
target_link_libraries(test_sphere PRIVATE luisa::render)
# test_sphere: CPU icosphere subdivision (uses ThreadPool + SphereGeometry).
add_test(NAME test_sphere COMMAND test_sphere)
set_tests_properties(test_sphere PROPERTIES TIMEOUT 120 LABELS integration)

add_executable(test_scene_node_tag test_scene_node_tag.cpp)
target_link_libraries(test_scene_node_tag PRIVATE luisa-render-sdl)
# test_scene_node_tag: integration tests for SDL tag parsing and SceneDesc API.
add_test(NAME test_scene_node_tag COMMAND test_scene_node_tag)
set_tests_properties(test_scene_node_tag PROPERTIES TIMEOUT 30 LABELS integration)

# test_alias_method: runs 1 B sampling iterations – accurate statistical benchmark
# but intentionally excluded from regular CTest to keep CI wall-time low.
# Run manually with: ./test_alias_method
