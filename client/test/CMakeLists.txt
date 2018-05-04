# Required header
cmake_minimum_required(VERSION 3.7)
project(client_test)

# Download dependencies
hunter_add_package(GTest)
hunter_add_package(Boost)

# Find dependencies
find_package(GTest CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED system filesystem)

# Add the executable
add_executable(client_test
        ../../util/include/dropboxUtil.hpp
        ../../util/src/dropboxUtil.cpp
        ../include/dropboxClient.hpp
        ../src/dropboxClient.cpp
        client_test.cpp)

# Target libraries
target_link_libraries(client_test PUBLIC GTest::main Boost::system Boost::filesystem)
