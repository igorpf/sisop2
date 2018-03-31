# Required header
cmake_minimum_required(VERSION 3.7)
project(util_test)

# Download dependencies
hunter_add_package(GTest)

# Find dependencies
find_package(GTest CONFIG REQUIRED)

# Add the executable
add_executable(util_test util_test.cpp)

# Target libraries
target_link_libraries(util_test PUBLIC GTest::main)
