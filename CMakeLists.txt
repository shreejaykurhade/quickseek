cmake_minimum_required(VERSION 3.16)

project(quickseek
  VERSION   0.1.0
  LANGUAGES CXX
  DESCRIPTION "Fast in-memory file search with ranked results"
  HOMEPAGE_URL "https://github.com/shreejaykurhade/quickseek"
)

# Options
option(QUICKSEEK_BUILD_TESTS    "Build the CTest test suite"         ON)
option(QUICKSEEK_ENABLE_TIDY    "Run clang-tidy during the build"    OFF)
option(QUICKSEEK_ENABLE_FORMAT  "Add a 'format' target (clang-format)" ON)

# C++ standard
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Optional: clang-tidy 
if(QUICKSEEK_ENABLE_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
  if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY
      ${CLANG_TIDY_EXE}
      --config-file=${CMAKE_SOURCE_DIR}/config/.clang-tidy
    )
    message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE}")
  else()
    message(WARNING "QUICKSEEK_ENABLE_TIDY=ON but clang-tidy was not found")
  endif()
endif()

# Library 
add_library(quickseek
  src/format.cpp
  src/index.cpp
  src/search.cpp
)
add_library(quickseek::quickseek ALIAS quickseek)

target_include_directories(quickseek
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(quickseek PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(quickseek PRIVATE /W4 /WX)
else()
  target_compile_options(quickseek PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# CLI executable
add_executable(quickseek_cli tools/quickseek_cli.cpp)
target_link_libraries(quickseek_cli PRIVATE quickseek)
set_target_properties(quickseek_cli PROPERTIES OUTPUT_NAME quickseek)

# Tests
if(QUICKSEEK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

# Optional: clang-format target
if(QUICKSEEK_ENABLE_FORMAT)
  find_program(CLANG_FORMAT_EXE NAMES clang-format)
  if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_SOURCE_FILES
      ${PROJECT_SOURCE_DIR}/src/*.cpp
      ${PROJECT_SOURCE_DIR}/include/*.h
      ${PROJECT_SOURCE_DIR}/tools/*.cpp
      ${PROJECT_SOURCE_DIR}/test/*.cpp
      ${PROJECT_SOURCE_DIR}/test/support/*.h
    )
    add_custom_target(format
      COMMAND ${CLANG_FORMAT_EXE}
        --style=file:${PROJECT_SOURCE_DIR}/config/.clang-format
        -i ${ALL_SOURCE_FILES}
      COMMENT "Running clang-format on all sources"
      VERBATIM
    )
    add_custom_target(format-check
      COMMAND ${CLANG_FORMAT_EXE}
        --style=file:${PROJECT_SOURCE_DIR}/config/.clang-format
        --dry-run --Werror ${ALL_SOURCE_FILES}
      COMMENT "Checking clang-format compliance"
      VERBATIM
    )
  else()
    message(STATUS "clang-format not found — 'format' target unavailable")
  endif()
endif()

# Install rules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS quickseek quickseek_cli
  EXPORT  quickseekTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/quickseek
  FILES_MATCHING PATTERN "*.h"
)

# CMake package config — lets downstream do find_package(quickseek)
install(EXPORT quickseekTargets
  FILE      quickseekTargets.cmake
  NAMESPACE quickseek::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickseek
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/quickseekConfigVersion.cmake"
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/quickseekConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/quickseekConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickseek
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/quickseekConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/quickseekConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/quickseek
)

# CPack packaging 
set(CPACK_PACKAGE_NAME          "quickseek")
set(CPACK_PACKAGE_VERSION       ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR        "quickseek contributors")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README  "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_GENERATOR             "ZIP;TGZ")
include(CPack)
