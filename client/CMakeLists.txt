# Required header
cmake_minimum_required(VERSION 3.7)
project(dropboxClient)

# Download dependencies
hunter_add_package(Boost COMPONENTS program_options system filesystem)
hunter_add_package(spdlog)

# Find dependencies
find_package(Boost CONFIG REQUIRED program_options system filesystem)
find_package(spdlog CONFIG REQUIRED)

# Add the executable
add_executable(dropboxClient
        ../util/include/dropboxUtil.hpp
        ../util/src/dropboxUtil.cpp
        include/dropboxClient.hpp
        src/dropboxClient.cpp
        src/main.cpp)

# Target libraries
target_link_libraries(dropboxClient PUBLIC Boost::program_options Boost::system Boost::filesystem spdlog::spdlog pthread)

# Add the test subdirectory
add_subdirectory(test)
