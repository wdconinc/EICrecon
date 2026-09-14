cmake_minimum_required(VERSION 3.16)
project(EICrecon_WASM)

include(FetchContent)
FetchContent_Declare(
  podio
  GIT_REPOSITORY https://github.com/AIDASoft/podio.git
  GIT_TAG v01-08)

set(ENABLE_ROOT
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_SIO
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_TESTING
    OFF
    CACHE BOOL "" FORCE)
set(podio_BUILD_DOXYGEN
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(podio)

add_executable(eicrecon_shell hello.cpp)
target_link_options(eicrecon_shell PRIVATE -lembind -O3)
target_link_libraries(eicrecon_shell PRIVATE podio::podio)
