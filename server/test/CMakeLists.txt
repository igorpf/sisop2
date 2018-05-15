# Required header
cmake_minimum_required(VERSION 3.5)
project(server_test)

# Download dependencies
hunter_add_package(GTest)
hunter_add_package(Boost)

# Find dependencies
find_package(GTest CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED system filesystem)

# Add the executable
add_executable(server_test
        ../../util/include/dropboxUtil.hpp
        ../../util/src/dropboxUtil.cpp
        ../../util/include/File.hpp
        ../../util/src/File.cpp
        ../include/dropboxServer.hpp
        ../src/dropboxServer.cpp
        server_test.cpp)

# Target libraries
target_link_libraries(server_test PUBLIC GTest::main Boost::system Boost::filesystem)
