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
    src/LiveServer.cpp
    src/actions/Add.cpp
    src/actions/GetLevelString.cpp
    src/actions/GetSelectedObjects.cpp
    src/actions/IGetObjectsAction.cpp
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
    GIT_TAG 7a7bfa1
    OPTIONS "WITH_LIBSODIUM OFF"
    "WITH_PERF_TOOL OFF"
    "WITH_DOCS OFF"
    "WITH_TESTS OFF"
    "WITH_TOOLS OFF"
    "WITH_LIBUNWIND OFF"
    "WITH_SYSTEMD OFF"
    "WITH_OPENPGM OFF"
    "WITH_DRAFTS OFF"
    "WITH_CURVE OFF"
    "WITH_LZ4 OFF"
    "WITH_GSSAPI OFF"
    "WITH_LIBSODIUM OFF"
    "WITH_LIBZMQ_STATIC ON"
    "WITH_LIBZMQ_SHARED OFF"
    "WITH_TLS OFF"
    "WITH_VALGRIND OFF"
    "WITH_ASAN OFF"
    "WITH_UBSAN OFF"
    "WITH_MSAN OFF"
    "WITH_TSAN OFF"
    "WITH_CPACK OFF"
    "BUILD_SHARED OFF"
    "BUILD_TESTS OFF"
    "ZMQ_BUILD_TESTS OFF"
    "ZMQ_STATIC ON"
)


CPMAddPackage(
    NAME cppzmq
    GITHUB_REPOSITORY zeromq/cppzmq
    GIT_TAG v4.10.0
    OPTIONS
    "CPPZMQ_BUILD_TESTS OFF" 
)

CPMAddPackage("gh:zhihaoy/nontype_functional#bdb0987")

target_link_libraries(${PROJECT_NAME} ixwebsocket cppzmq-static std23::nontype_functional)

target_include_directories(${PROJECT_NAME} PRIVATE src)
