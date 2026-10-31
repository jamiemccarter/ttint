cmake_minimum_required(VERSION 3.21)

include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG d44d4f6e66232d716af82f00a063759e9d0e50d6
)
FetchContent_MakeAvailable(doctest)

add_executable(ttint_tests
    test_main.cpp
    test_style.cpp
    test_colors.cpp
    test_segments.cpp
    test_render.cpp
    test_platform.cpp
    test_stream.cpp
    test_ostream.cpp
    test_header_textstyle.cpp
    test_header_ostreamstyle.cpp
    test_header_applehig.cpp
    test_header_pico8.cpp
    test_header_curiosities.cpp
    test_header_ansi16.cpp
    test_palettes.cpp
)

target_link_libraries(ttint_tests PRIVATE TTint::ttint doctest::doctest)
target_compile_features(ttint_tests PRIVATE cxx_std_20)
ttint_enable_warnings(ttint_tests)

enable_testing()
add_test(NAME ttint_all_tests COMMAND ttint_tests)

# [[nodiscard]] regression: each case in nodiscard_fixture.cpp ignores one
# result and must fail to build; the control case must build. Without the
# control, a broken include would make every case "pass".
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(TTINT_NODISCARD_CASES 0 1 2 3 4 5 6 7 8 9 10)
    foreach(case IN LISTS TTINT_NODISCARD_CASES)
        set(target ttint_nodiscard_case_${case})
        add_executable(${target} EXCLUDE_FROM_ALL nodiscard_fixture.cpp)
        target_link_libraries(${target} PRIVATE TTint::ttint)
        target_compile_features(${target} PRIVATE cxx_std_20)
        target_compile_definitions(${target} PRIVATE TTINT_NODISCARD_CASE=${case})
        target_compile_options(${target} PRIVATE -Werror=unused-result)
        add_test(NAME ttint_nodiscard_case_${case}
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${target} --config $<CONFIG>)
        if(NOT case EQUAL 0)
            set_tests_properties(ttint_nodiscard_case_${case} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endif()

# Installs the library to a scratch prefix, then configures, builds and runs a
# separate project that consumes it through find_package(TTint). This proves the
# installed package files, not just the in-tree target, work for a consumer.
# CMAKE_NO_SYSTEM_FROM_IMPORTED makes the package include directory a plain -I:
# Apple clang searches -isystem directories after /usr/local/include, so a TTint
# copied there earlier would otherwise shadow the package under test.
set(TTINT_PACKAGE_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/package_prefix)
set(TTINT_PACKAGE_BUILD ${CMAKE_CURRENT_BINARY_DIR}/package_consumer)
add_test(NAME ttint_package_install
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${TTINT_PACKAGE_PREFIX} --config $<CONFIG>)
add_test(NAME ttint_package_configure
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_SOURCE_DIR}/package_consumer -B ${TTINT_PACKAGE_BUILD}
            -DCMAKE_PREFIX_PATH=${TTINT_PACKAGE_PREFIX} -DCMAKE_BUILD_TYPE=$<CONFIG>
            -DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON)
add_test(NAME ttint_package_build
    COMMAND ${CMAKE_COMMAND} --build ${TTINT_PACKAGE_BUILD} --config $<CONFIG>)
add_test(NAME ttint_package_run
    COMMAND ${CMAKE_COMMAND} -DCONSUMER_DIR=${TTINT_PACKAGE_BUILD} -DCONFIG=$<CONFIG>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/package_consumer/run.cmake)
set_tests_properties(ttint_package_install PROPERTIES FIXTURES_SETUP ttint_package_prefix)
set_tests_properties(ttint_package_configure PROPERTIES FIXTURES_REQUIRED ttint_package_prefix FIXTURES_SETUP ttint_package_configured)
set_tests_properties(ttint_package_build PROPERTIES FIXTURES_REQUIRED ttint_package_configured FIXTURES_SETUP ttint_package_built)
set_tests_properties(ttint_package_run PROPERTIES FIXTURES_REQUIRED ttint_package_built)

if(TTINT_STYLE_CHECKS AND Python3_FOUND)
    # Exit status 77 means a tool such as clang-format is not installed; report
    # that as SKIPPED rather than a pass. CI sets TTINT_REQUIRE_TOOLS=1 so a
    # missing tool fails instead.
    add_test(NAME ttint_style COMMAND ${TTINT_STYLE_COMMAND}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
    set_tests_properties(ttint_style PROPERTIES SKIP_RETURN_CODE 77)

    # Proves each style check fires on a deliberate violation (see tests/style).
    # Cases that need a missing tool are skipped, which the script reports as 77.
    add_test(NAME ttint_style_selftest
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/style/test_check_style.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
    set_tests_properties(ttint_style_selftest PROPERTIES SKIP_RETURN_CODE 77)
endif()
