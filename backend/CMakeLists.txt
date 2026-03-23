cmake_minimum_required(VERSION 3.16)
project(secsgem_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(secsgem_backend
    src/main.cpp
    src/http_server.cpp
)

target_include_directories(secsgem_backend PRIVATE include)

if (UNIX AND NOT APPLE)
    target_compile_definitions(secsgem_backend PRIVATE SECSGEM_POSIX_SERVER=1)
endif()

