# Include hunter package manager and set project name
cmake_minimum_required(VERSION 3.7)
include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.20.31.tar.gz"
        SHA1 "65c8f2292566aedb2fe10e08e09c74dda8e5d07a"
)
project(dropbox)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -lstdc++fs -std=c++11 -lpthread")
# Add the subprojects
add_subdirectory(util)
add_subdirectory(client)
add_subdirectory(server)
