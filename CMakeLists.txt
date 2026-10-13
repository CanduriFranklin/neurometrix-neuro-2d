cmake_minimum_required(VERSION 3.30)
project(neuro_2d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(RdKafka CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(DCMTK CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

# ---------------------------------------------------------------------------
# Target: neuro-2d
# ---------------------------------------------------------------------------
add_executable(neuro-2d
    main.cpp
    application/use_cases/process_study_use_case.cpp
    infrastructure/adapters/kafka/kafka_consumer_adapter.cpp
    infrastructure/adapters/kafka/kafka_producer_adapter.cpp
    infrastructure/adapters/dicom/dicom_loader_adapter.cpp
    infrastructure/adapters/postgres/postgres_repository_adapter.cpp
    infrastructure/adapters/config/config_loader.cpp
    infrastructure/adapters/logging/logger.cpp
    infrastructure/adapters/metrics/metrics_server.cpp
    infrastructure/adapters/metrics/metrics_registry.cpp
    infrastructure/adapters/security/tls_context.cpp
)

target_include_directories(neuro-2d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(neuro-2d PRIVATE
    RdKafka::rdkafka
    RdKafka::rdkafka++
    PostgreSQL::PostgreSQL
    DCMTK::DCMTK
    OpenSSL::Crypto
    OpenSSL::SSL
    spdlog::spdlog
    prometheus-cpp::core
    prometheus-cpp::pull
    httplib::httplib
    Eigen3::Eigen
    OpenMP::OpenMP_CXX
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)

if(MSVC)
    target_compile_options(neuro-2d PRIVATE /openmp)
endif()

if(MSVC)
    target_compile_options(neuro-2d PRIVATE /openmp)
endif()
