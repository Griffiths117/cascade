# Tests. Framework: Catch2 v3 (see ADR 0004), fetched at configure time.
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Catch2)

# --- dependency-free smoke test (kept for the scaffold) ---
add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE cascade::cascade)
add_test(NAME smoke COMMAND smoke_test)

# --- Catch2-based unit tests ---
file(GLOB_RECURSE CASCADE_UNIT_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp)
if(CASCADE_UNIT_SOURCES)
    add_executable(cascade_tests ${CASCADE_UNIT_SOURCES})
    target_link_libraries(cascade_tests PRIVATE cascade::cascade Catch2::Catch2WithMain)
    add_test(NAME cascade_tests COMMAND cascade_tests)
endif()

# Every public header must compile on its own, with only its own includes.
set(CASCADE_PUBLIC_HEADERS
    scalar matrix standard calibration uncertainty measurement version)

foreach(header IN LISTS CASCADE_PUBLIC_HEADERS)
    set(src ${CMAKE_CURRENT_BINARY_DIR}/header_${header}.cpp)
    file(WRITE ${src} "#include \"cascade/${header}.hpp\"\nint main() { return 0; }\n")
    add_executable(header_check_${header} ${src})
    target_link_libraries(header_check_${header} PRIVATE cascade::cascade)
    add_test(NAME header_${header} COMMAND header_check_${header})
endforeach()

set(src ${CMAKE_CURRENT_BINARY_DIR}/header_io_sdatb.cpp)
file(WRITE ${src} "#include \"cascade/io/sdatb.hpp\"\nint main() { return 0; }\n")
add_executable(header_check_io_sdatb ${src})
target_link_libraries(header_check_io_sdatb PRIVATE cascade::cascade)
add_test(NAME header_io_sdatb COMMAND header_check_io_sdatb)
