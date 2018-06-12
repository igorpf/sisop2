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
        ../util/src/dropboxUtil.cpp
        ../util/include/File.hpp
        ../util/src/File.cpp
        include/dropboxServer.hpp
        src/dropboxServer.cpp
        src/main.cpp
        src/ClientThread.cpp
        include/ClientThread.hpp
        src/ClientThreadPool.cpp
        include/ClientThreadPool.hpp
        src/main.cpp
        ../util/src/LoggerFactory.cpp
        ../util/include/LoggerFactory.hpp ../util/include/logger_wrapper.hpp src/server_login_parser.cpp include/server_login_parser.hpp)

# Target libraries
target_link_libraries(dropboxServer PUBLIC Boost::system Boost::program_options Boost::filesystem spdlog::spdlog pthread -static-libstdc++)

# Compile options
target_compile_options(dropboxServer PUBLIC -std=c++11 -lpthread)

# Add the test subdirectory
add_subdirectory(test)
