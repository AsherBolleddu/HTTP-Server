cmake_minimum_required(VERSION 3.17)

project(http-server)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(http-server ${SOURCE_FILES})
target_compile_options(http-server PRIVATE -Wall -Wextra -Wconversion -Wsign-conversion -Werror)
target_link_libraries(http-server PRIVATE Threads::Threads ZLIB::ZLIB)
