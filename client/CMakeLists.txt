# Required header
cmake_minimum_required(VERSION 3.5)
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
        ../util/include/lock_guard.hpp
        ../util/include/pthread_wrapper.hpp
        ../util/include/LoggerFactory.hpp
        ../util/src/dropboxUtil.cpp
        ../util/src/File.cpp
        ../util/src/table_printer.cpp
        ../util/src/LoggerFactory.cpp
        include/iclient.hpp
        include/login_command_parser.hpp
        include/shell_command_parser.hpp
        include/dropboxClient.hpp
        include/shell.hpp
        include/sync_thread.hpp
        include/file_watcher.hpp
        src/login_command_parser.cpp
        src/shell_command_parser.cpp
        src/dropboxClient.cpp
        src/shell.cpp
        src/sync_thread.cpp
        src/file_watcher.cpp
        src/main.cpp ../util/include/logger_wrapper.hpp src/FrontendThread.cpp include/FrontendThread.hpp)

# Target libraries
target_link_libraries(dropboxClient PUBLIC Boost::program_options Boost::boost Boost::system Boost::filesystem spdlog::spdlog pthread -static-libstdc++)

# Compile options
target_compile_options(dropboxClient PUBLIC -std=c++11 -lpthread)

# Add the test subdirectory
add_subdirectory(test)
