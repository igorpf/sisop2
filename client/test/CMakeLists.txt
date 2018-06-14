# Required header
cmake_minimum_required(VERSION 3.5)
project(client_test)

# Download dependencies
hunter_add_package(GTest)
hunter_add_package(Boost COMPONENTS program_options system filesystem)

# Find dependencies
find_package(GTest CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED program_options system filesystem)

# Add the executable
add_executable(client_test
        ../../util/include/dropboxUtil.hpp
        ../../util/include/string_formatter.hpp
        ../../util/include/File.hpp
        ../../util/include/table_printer.hpp
        ../../util/src/dropboxUtil.cpp
        ../../util/src/File.cpp
        ../../util/src/table_printer.cpp
        ../include/login_command_parser.hpp
        ../include/shell_command_parser.hpp
        ../include/dropboxClient.hpp
        ../include/shell.hpp
        ../src/login_command_parser.cpp
        ../src/shell_command_parser.cpp
        ../src/dropboxClient.cpp
        ../src/shell.cpp
        client_test.cpp
        ../../util/src/LoggerFactory.cpp
        ../../util/include/LoggerFactory.hpp ../../util/include/logger_wrapper.hpp ../src/FrontendThread.cpp ../include/FrontendThread.hpp)

# Target libraries
target_link_libraries(client_test PUBLIC GTest::main Boost::program_options Boost::boost Boost::system Boost::filesystem -static-libstdc++)

# Compile options
target_compile_options(client_test PUBLIC -std=c++11 -lpthread)
