# Required header
cmake_minimum_required(VERSION 3.7)
project(util_test)

# Download dependencies
hunter_add_package(GTest)
hunter_add_package(Boost)

# Find dependencies
find_package(GTest CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED system filesystem)

# Add the executable
add_executable(util_test
        ../include/dropboxUtil.hpp
        ../src/dropboxUtil.cpp
        ../include/File.hpp
        ../src/File.cpp
        util_test.cpp)

# Target libraries
target_link_libraries(util_test PUBLIC GTest::main Boost::system Boost::filesystem)
