cmake_minimum_required(VERSION 3.21)
include(CPM.cmake)

project(bro2json LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} src/main.cpp)

CPMAddPackage("gh:geode-sdk/broma#0183881")
CPMAddPackage("gh:geode-sdk/json#691dcbf")

target_link_libraries(${PROJECT_NAME} Broma mat-json)