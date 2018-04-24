# Required header
cmake_minimum_required(VERSION 3.7)
project(dropboxUtil)

# Download dependencies
hunter_add_package(Boost COMPONENTS system filesystem)

# Find dependencies
find_package(Boost CONFIG REQUIRED system filesystem)

# Add the library
add_library(dropboxUtil
        include/dropboxUtil.hpp
        src/dropboxUtil.cpp)

# Target libraries
target_link_libraries(dropboxUtil PUBLIC Boost::system Boost::filesystem)

# Add the test subdirectory
add_subdirectory(test)
