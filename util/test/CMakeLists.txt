# Required header
cmake_minimum_required(VERSION 3.5)
project(util_test)

# Download dependencies
hunter_add_package(GTest)
hunter_add_package(Boost)

# Find dependencies
find_package(GTest CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED system filesystem)

# Add the executable
add_executable(util_test
        ../include/dropboxUtil.hpp
        ../include/string_formatter.hpp
        ../include/File.hpp
        ../include/table_printer.hpp
        ../src/dropboxUtil.cpp
        ../src/File.cpp
        ../src/table_printer.cpp
        util_test.cpp)

# Target libraries
target_link_libraries(util_test PUBLIC GTest::main Boost::system Boost::filesystem -static-libstdc++)

# Compile options
target_compile_options(util_test PUBLIC -std=c++11 -lpthread)
