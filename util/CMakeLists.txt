# Required header
cmake_minimum_required(VERSION 3.7)
project(dropboxUtil)

# Add the library
add_library(dropboxUtil
        include/dropboxUtil.hpp
        src/dropboxUtil.cpp)

# Add the test subdirectory
add_subdirectory(test)
