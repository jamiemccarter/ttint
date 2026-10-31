cmake_minimum_required(VERSION 3.21)
project(TTintPackageConsumer LANGUAGES CXX)

find_package(TTint 1.0.0 CONFIG REQUIRED)

add_executable(ttint_package_consumer main.cpp)
target_compile_features(ttint_package_consumer PRIVATE cxx_std_20)
target_link_libraries(ttint_package_consumer PRIVATE TTint::ttint)
