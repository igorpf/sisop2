# Required header
cmake_minimum_required(VERSION 3.7)
project(client_test)

# Download dependencies
hunter_add_package(GTest)

# Find dependencies
find_package(GTest CONFIG REQUIRED)

# Add the executable
add_executable(client_test
        ../include/dropboxClient.hpp
        ../src/dropboxClient.cpp
        ../src/main.cpp
        client_test.cpp)

# Target libraries
target_link_libraries(client_test PUBLIC GTest::main)
