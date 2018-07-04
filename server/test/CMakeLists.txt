# Required header
cmake_minimum_required(VERSION 3.5)
project(server_test)

# Download dependencies
hunter_add_package(GTest)
hunter_add_package(Boost)

# Find dependencies
find_package(GTest CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED system filesystem program_options)

# Add the executable
add_executable(server_test
        ../../util/include/dropboxUtil.hpp
        ../../util/include/File.hpp
        ../../util/include/logger_wrapper.hpp
        ../../util/include/LoggerFactory.hpp
        ../../util/include/pthread_wrapper.hpp
        ../../util/src/dropboxUtil.cpp
        ../../util/src/File.cpp
        ../../util/src/LoggerFactory.cpp
        ../include/server_login_parser.hpp
        ../include/ClientThread.hpp
        ../include/ClientThreadPool.hpp
        ../include/PrimaryServerConnectivityDetectorThread.hpp
        ../include/backup_sync_thread.hpp
        ../include/backup_file_sync_thread.hpp
        ../include/dropboxServer.hpp
        ../src/server_login_parser.cpp
        ../src/ClientThread.cpp
        ../src/ClientThreadPool.cpp
        ../src/PrimaryServerConnectivityDetectorThread.cpp
        ../src/backup_sync_thread.cpp
        ../src/backup_file_sync_thread.cpp
        ../src/dropboxServer.cpp
        server_test.cpp)

# Target libraries
target_link_libraries(server_test PUBLIC GTest::main Boost::system Boost::program_options Boost::filesystem -static-libstdc++)

# Compile options
target_compile_options(server_test PUBLIC -std=c++11 -lpthread)
