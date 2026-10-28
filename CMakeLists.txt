cmake_minimum_required(VERSION 3.24)

project(cascade
    VERSION 0.0.1
    DESCRIPTION "S-parameter measurements with traceable uncertainty"
    HOMEPAGE_URL "https://example.invalid/cascade"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(CASCADE_BUILD_TESTS      "Build the test suite"                ON)
option(CASCADE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(CASCADE_ENABLE_SANITIZERS "Build with ASan/UBSan (debug builds)" OFF)
option(CASCADE_INSTALL          "Generate install rules"              ON)

# ---------------------------------------------------------------------------
# Language settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(Eigen3 3.4 REQUIRED NO_MODULE)

# ---------------------------------------------------------------------------
# Library
# ---------------------------------------------------------------------------
add_library(cascade
    src/version.cpp
    src/io.cpp
)
add_library(cascade::cascade ALIAS cascade)

target_include_directories(cascade
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cascade PUBLIC Eigen3::Eigen)

target_compile_features(cascade PUBLIC cxx_std_20)

target_compile_definitions(cascade PRIVATE
    CASCADE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    CASCADE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    CASCADE_VERSION_PATCH=${PROJECT_VERSION_PATCH})

# Warnings: public headers are compiled by consumers too, so they must be clean.
set(CASCADE_WARNINGS
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
    -Woverloaded-virtual -Wnull-dereference -Wdouble-promotion)
target_compile_options(cascade PRIVATE ${CASCADE_WARNINGS})
if(CASCADE_WARNINGS_AS_ERRORS)
    target_compile_options(cascade PRIVATE -Werror)
endif()

if(CASCADE_ENABLE_SANITIZERS)
    target_compile_options(cascade PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(cascade PUBLIC -fsanitize=address,undefined)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(CASCADE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
if(CASCADE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS cascade
        EXPORT cascadeTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(DIRECTORY include/cascade
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT cascadeTargets
        FILE cascadeTargets.cmake
        NAMESPACE cascade::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cascade)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/cascadeConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMinorVersion)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cascadeConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cascade)
endif()
