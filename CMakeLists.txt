cmake_minimum_required(VERSION 3.16)

project(quickseek VERSION 0.1.0 LANGUAGES CXX)

option(QUICKSEEK_BUILD_TESTS "Build QuickSeek tests" ON)

add_library(quickseek
  src/format.cpp
  src/index.cpp
  src/search.cpp
)

target_compile_features(quickseek PUBLIC cxx_std_17)

target_include_directories(quickseek
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

if(MSVC)
  target_compile_options(quickseek PRIVATE /W4)
else()
  target_compile_options(quickseek PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(quickseek_cli tools/quickseek_cli.cpp)
target_link_libraries(quickseek_cli PRIVATE quickseek)
set_target_properties(quickseek_cli PROPERTIES OUTPUT_NAME quickseek)

if(QUICKSEEK_BUILD_TESTS)
  enable_testing()
  add_executable(quickseek_tests test/quickseek_tests.cpp)
  target_link_libraries(quickseek_tests PRIVATE quickseek)
  add_test(NAME quickseek_tests COMMAND quickseek_tests)
endif()
