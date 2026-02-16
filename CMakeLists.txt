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
    #src/FlexibleServer.cpp
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

CPMAddPackage(
  NAME msft_proxy4
  GIT_TAG 4.0.0 # or above
  GIT_REPOSITORY https://github.com/microsoft/proxy.git
  OPTIONS "BUILD_TESTING OFF; PROJECT_IS_TOP_LEVEL OFF"
)

CPMAddPackage("gh:stephenberry/glaze@6.0.3")

CPMAddPackage(
  GITHUB_REPOSITORY zaphoyd/websocketpp
  GIT_TAG 4dfe1be
  OPTIONS "BUILD_EXAMPLES OFF;BUILD_SHARED OFF"
)


target_link_libraries(${PROJECT_NAME} websocketpp glaze::glaze msft_proxy4::proxy)

