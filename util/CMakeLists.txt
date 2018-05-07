# Required header
cmake_minimum_required(VERSION 3.7)
project(dropboxUtil)

# Download dependencies
hunter_add_package(Boost COMPONENTS system filesystem)
hunter_add_package(spdlog)

# Find dependencies
find_package(Boost CONFIG REQUIRED system filesystem)
find_package(spdlog CONFIG REQUIRED)

# Add the library
add_library(dropboxUtil
        include/dropboxUtil.hpp
        src/dropboxUtil.cpp
        include/File.hpp
        src/File.cpp)

# Target libraries
target_link_libraries(dropboxUtil PUBLIC Boost::system Boost::filesystem spdlog::spdlog pthread)

# Add the test subdirectory
add_subdirectory(test)
