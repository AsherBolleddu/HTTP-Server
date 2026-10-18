cmake_minimum_required(VERSION 3.21)

project(http-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp src/*.hpp)

add_executable(http-server ${SOURCE_FILES})

target_compile_options(http-server PRIVATE
    -Wall
    -Wextra
    -Weffc++
    -Wconversion
    -Wsign-conversion
    -pedantic-errors
    -Werror
)

target_link_libraries(http-server PRIVATE Threads::Threads ZLIB::ZLIB)
