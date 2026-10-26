# Tests are deliberately dependency-free for now: plain executables with a
# return code, registered with CTest. A framework (Catch2 or GoogleTest) can be
# brought in when there is more than a smoke test to run, and the choice is
# recorded as an ADR rather than made by accident.

add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE cascade::cascade)
add_test(NAME smoke COMMAND smoke_test)

# Every public header must compile on its own, with only its own includes.
# Each of these translation units includes exactly one header.
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
