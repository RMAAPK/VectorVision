cmake_minimum_required(VERSION 3.20)
project(VectorVision VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

add_library(svgv_format src/svgv_format.cpp)
target_include_directories(svgv_format PUBLIC include)
target_link_libraries(svgv_format PUBLIC ZLIB::ZLIB)

add_executable(svgv_tests tests/test_codec.cpp)
target_link_libraries(svgv_tests PRIVATE svgv_format)
add_executable(svgv_bench bench/benchmark.cpp bench/image_io.cpp)
target_include_directories(svgv_bench PRIVATE bench)
target_link_libraries(svgv_bench PRIVATE svgv_format PNG::PNG JPEG::JPEG ZLIB::ZLIB)

enable_testing()
add_test(NAME codec_roundtrip_and_corruption COMMAND svgv_tests)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  foreach(target svgv_format svgv_tests svgv_bench)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endforeach()
endif()
