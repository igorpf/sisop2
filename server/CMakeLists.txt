# Required header
cmake_minimum_required(VERSION 3.7)
project(dropboxServer)

# Add the executable
add_executable(dropboxServer
        include/dropboxServer.hpp
        src/dropboxServer.cpp
        src/main.cpp)

# Add the test subdirectory
add_subdirectory(test)