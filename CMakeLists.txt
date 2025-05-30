cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
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

FetchContent_Declare(
  proxy
  GIT_REPOSITORY https://github.com/microsoft/proxy.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(proxy)

target_link_libraries(${PROJECT_NAME} ixwebsocket msft_proxy)

target_include_directories(${PROJECT_NAME} PRIVATE src)
