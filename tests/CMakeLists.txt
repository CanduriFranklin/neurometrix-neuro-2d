find_package(GTest CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------
add_executable(neuro-2d-tests
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_hounsfield.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_physics_filters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_morphological_ops.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_hessian_analysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_bilateral_symmetry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_entropy_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_finding_extraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_contracts_serde.cpp
)
target_include_directories(neuro-2d-tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(neuro-2d-tests PRIVATE GTest::gtest GTest::gtest_main Eigen3::Eigen OpenMP::OpenMP_CXX)
if(MSVC)
    target_compile_options(neuro-2d-tests PRIVATE /openmp)
endif()
include(GoogleTest)
gtest_discover_tests(neuro-2d-tests)

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------
function(add_neuro_benchmark NAME SRC)
    add_executable(${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/performance/${SRC})
    target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(${NAME} PRIVATE Eigen3::Eigen OpenMP::OpenMP_CXX)
    if(MSVC)
        target_compile_options(${NAME} PRIVATE /openmp)
    endif()
endfunction()

add_neuro_benchmark(benchmark_pipeline   benchmark_pipeline.cpp)
add_neuro_benchmark(benchmark_throughput benchmark_throughput.cpp)
add_neuro_benchmark(benchmark_memory     benchmark_memory.cpp)