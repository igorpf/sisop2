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
        ../util/include/string_formatter.hpp
        ../util/include/File.hpp
        ../util/include/table_printer.hpp
        ../util/src/dropboxUtil.cpp
        ../util/src/File.cpp
        ../util/src/table_printer.cpp
        include/iclient.hpp
        include/login_command_parser.hpp
        include/shell_command_parser.hpp
        include/dropboxClient.hpp
        include/shell.hpp
        src/login_command_parser.cpp
        src/shell_command_parser.cpp
        src/dropboxClient.cpp
        src/shell.cpp
        src/main.cpp)

# Target libraries
target_link_libraries(dropboxClient PUBLIC Boost::program_options Boost::boost Boost::system Boost::filesystem spdlog::spdlog pthread)

# Add the test subdirectory
add_subdirectory(test)
