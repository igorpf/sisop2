# Required header
cmake_minimum_required(VERSION 3.7)
project(dropboxServer)

# Download dependencies
hunter_add_package(Boost COMPONENTS system filesystem)

# Find dependencies
find_package(Boost CONFIG REQUIRED system filesystem)

# Add the executable
add_executable(dropboxServer
        ../util/include/dropboxUtil.hpp
        ../util/src/dropboxUtil.cpp
        include/dropboxServer.hpp
        src/dropboxServer.cpp
        src/main.cpp)

# Target libraries
target_link_libraries(dropboxServer PUBLIC Boost::system Boost::filesystem)

# Add the test subdirectory
add_subdirectory(test)
