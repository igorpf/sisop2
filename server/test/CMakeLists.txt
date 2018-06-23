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
        ../../util/src/dropboxUtil.cpp
        ../../util/include/File.hpp
        ../../util/src/File.cpp
        ../include/dropboxServer.hpp
        ../src/dropboxServer.cpp
        server_test.cpp
        ../../util/src/LoggerFactory.cpp
        ../../util/include/LoggerFactory.hpp
        ../src/ClientThread.cpp
        ../include/ClientThread.hpp
        server_test.cpp
        ../src/ClientThreadPool.cpp
        ../include/ClientThreadPool.hpp ../../util/include/logger_wrapper.hpp ../src/server_login_parser.cpp ../include/server_login_parser.hpp ../src/PrimaryServerConnectivityDetectorThread.cpp ../include/PrimaryServerConnectivityDetectorThread.hpp)

# Target libraries
target_link_libraries(server_test PUBLIC GTest::main Boost::system Boost::program_options Boost::filesystem -static-libstdc++)

# Compile options
target_compile_options(server_test PUBLIC -std=c++11 -lpthread)
