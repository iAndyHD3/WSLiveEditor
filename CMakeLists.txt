cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)
set(USE_ZLIB OFF)

project(WSLiveEditor VERSION 1.0.0)

file(GLOB SOURCES
    src/main.cpp
    src/FlexibleServer.cpp
    src/LiveServer.cpp
    src/actions/Add.cpp
    src/actions/GetLevelString.cpp
    src/actions/GetSelectedObjects.cpp
    src/actions/Remove.cpp
    src/actions/RemoveSelected.cpp
)


add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})

CPMAddPackage("gh:machinezone/IXWebSocket#e03c0be")


CPMAddPackage(
  NAME libzmq
  GITHUB_REPOSITORY zeromq/libzmq
  GIT_TAG v4.3.5
  OPTIONS "CMAKE_CXX_FLAGS -w"
  OPTIONS "BUILD_TESTS OFF"
  OPTIONS "BUILD_SHARED OFF"
)
CPMAddPackage(
  NAME cppzmq
  GITHUB_REPOSITORY zeromq/cppzmq
  GIT_TAG v4.11.0
  OPTIONS "CPPZMQ_BUILD_TESTS OFF"
)

CPMAddPackage("gh:zhihaoy/nontype_functional#bdb0987")

target_link_libraries(${PROJECT_NAME} ixwebsocket cppzmq-static std23::nontype_functional)

target_include_directories(${PROJECT_NAME} PRIVATE src)
