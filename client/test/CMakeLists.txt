# Required header
cmake_minimum_required(VERSION 3.7)
project(client_test)

# Download dependencies
hunter_add_package(GTest)
hunter_add_package(Boost COMPONENTS program_options system thread)

# Find dependencies
find_package(GTest CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED program_options system thread)
find_package(Boost CONFIG REQUIRED system filesystem)

# Add the executable
add_executable(client_test
        ../../util/include/dropboxUtil.hpp
        ../../util/src/dropboxUtil.cpp
        ../../util/include/File.hpp
        ../../util/src/File.cpp
        ../include/dropboxClient.hpp
        ../include/command_parser.hpp
        ../src/dropboxClient.cpp
        ../src/command_parser.cpp
        client_test.cpp)

# Target libraries
target_link_libraries(client_test PUBLIC GTest::main)
target_link_libraries(client_test PUBLIC Boost::program_options Boost::boost Boost::system Boost::thread Boost::filesystem)
