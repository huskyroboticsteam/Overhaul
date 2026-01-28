cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Rover LANGUAGES CXX C)

# Dependencies
include(FetchContent)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG "master"
)
FetchContent_MakeAvailable(argparse)

add_executable(Rover Rover.cpp)

target_link_libraries(Rover argparse)