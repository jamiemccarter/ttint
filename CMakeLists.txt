cmake_minimum_required(VERSION 3.21)
project(TTint VERSION 1.0.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(TTINT_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/TTint")

option(TTINT_BUILD_EXAMPLES "Build TTint example programs" OFF)
option(TTINT_BUILD_TESTS "Build TTint test suite" OFF)
# Style targets are for contributors; a project that pulls TTint in as a
# subproject does not get them unless it asks.
option(TTINT_STYLE_CHECKS "Add style-check targets and tests (scripts/check_style.py)" ${PROJECT_IS_TOP_LEVEL})
option(TTINT_WARNINGS "Compile tests and examples with the project warning level" ON)
option(TTINT_WERROR "Treat warnings in tests and examples as errors" OFF)

# Applies the warning level the style guide requires. Tests and examples only;
# the INTERFACE target never forces flags on consumers.
function(ttint_enable_warnings target)
    if(NOT TTINT_WARNINGS)
        return()
    endif()
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 $<$<BOOL:${TTINT_WERROR}>:/WX>)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic $<$<BOOL:${TTINT_WERROR}>:-Werror>)
    endif()
endfunction()

if(TTINT_STYLE_CHECKS)
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_FOUND)
        set(TTINT_STYLE_COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_style.py)
        foreach(check IN ITEMS format tokens cases naming readme)
            add_custom_target(ttint_style_${check}
                COMMAND ${TTINT_STYLE_COMMAND} ${check}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                USES_TERMINAL)
        endforeach()
        add_custom_target(ttint_style_all
            COMMAND ${TTINT_STYLE_COMMAND} all
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            USES_TERMINAL)
    else()
        message(STATUS "TTint: Python 3 not found; style targets and tests disabled")
    endif()
endif()

add_library(ttint INTERFACE)
add_library(TTint::ttint ALIAS ttint)
target_include_directories(ttint INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(ttint INTERFACE cxx_std_20)

if(TTINT_BUILD_EXAMPLES)
    # Each README code block has a matching file here; keep them in sync
    # (scripts/check_readme_examples.py).
    set(TTINT_EXAMPLES
        status
        owned_strings
        reusable_styles
        iostreams
        color_policy
        color_formats
        named_colors
        apple_colors
        showcase
    )
    set(TTINT_EXAMPLE_TARGETS)
    foreach(example IN LISTS TTINT_EXAMPLES)
        add_executable(ttint_${example} examples/${example}.cpp)
        target_compile_features(ttint_${example} PRIVATE cxx_std_20)
        target_link_libraries(ttint_${example} PRIVATE TTint::ttint)
        ttint_enable_warnings(ttint_${example})
        list(APPEND TTINT_EXAMPLE_TARGETS ttint_${example})
    endforeach()

    install(TARGETS ${TTINT_EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
        COMPONENT examples
    )
endif()

if(TTINT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    PATTERN ".DS_Store" EXCLUDE
)
install(TARGETS ttint EXPORT TTintTargets)
install(EXPORT TTintTargets
    FILE TTintTargets.cmake
    NAMESPACE TTint::
    DESTINATION "${TTINT_INSTALL_CMAKEDIR}"
)

configure_package_config_file(
    cmake/TTintConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/TTintConfig.cmake"
    INSTALL_DESTINATION "${TTINT_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/TTintConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TTintConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TTintConfigVersion.cmake"
    DESTINATION "${TTINT_INSTALL_CMAKEDIR}"
)
install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/TTint")
