# Required header
cmake_minimum_required(VERSION 3.5)
project(dropboxServer)

# Download dependencies
hunter_add_package(Boost COMPONENTS system filesystem)
hunter_add_package(spdlog)

# Find dependencies
find_package(Boost CONFIG REQUIRED system filesystem program_options)
find_package(spdlog CONFIG REQUIRED)

# Add the executable
add_executable(dropboxServer
        ../util/include/dropboxUtil.hpp
        ../util/include/File.hpp
        ../util/include/logger_wrapper.hpp
        ../util/include/LoggerFactory.hpp
        ../util/include/pthread_wrapper.hpp
        ../util/src/dropboxUtil.cpp
        ../util/src/File.cpp
        ../util/src/LoggerFactory.cpp
        include/server_login_parser.hpp
        include/ClientThread.hpp
        include/ClientThreadPool.hpp
        include/PrimaryServerConnectivityDetectorThread.hpp
        include/backup_sync_thread.hpp
        include/backup_file_sync_thread.hpp
        include/dropboxServer.hpp
        src/server_login_parser.cpp
        src/ClientThread.cpp
        src/ClientThreadPool.cpp
        src/PrimaryServerConnectivityDetectorThread.cpp
        src/backup_sync_thread.cpp
        src/backup_file_sync_thread.cpp
        src/dropboxServer.cpp
        src/main.cpp)

# Target libraries
target_link_libraries(dropboxServer PUBLIC Boost::system Boost::program_options Boost::filesystem spdlog::spdlog pthread -static-libstdc++)

# Compile options
target_compile_options(dropboxServer PUBLIC -std=c++11 -lpthread)

# Add the test subdirectory
add_subdirectory(test)
