# Required header
cmake_minimum_required(VERSION 3.5)
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
        include/string_formatter.hpp
        include/File.hpp
        include/table_printer.hpp
        include/lock_guard.hpp
        src/dropboxUtil.cpp
        src/File.cpp
        src/table_printer.cpp)

# Target libraries
target_link_libraries(dropboxUtil PUBLIC Boost::system Boost::filesystem spdlog::spdlog pthread -static-libstdc++)

# Compile options
target_compile_options(dropboxUtil PUBLIC -std=c++11 -lpthread)

# Add the test subdirectory
add_subdirectory(test)
