cmake_minimum_required(VERSION 3.16)
project(EICrecon_WASM)

include(FetchContent)
FetchContent_Declare(
  podio
  GIT_REPOSITORY https://github.com/wdconinc/podio.git
  GIT_TAG wasm)

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

FetchContent_Declare(
  edm4hep
  GIT_REPOSITORY https://github.com/wdconinc/EDM4hep.git
  GIT_TAG fix-dictionary-dependency)
FetchContent_MakeAvailable(edm4hep)

add_executable(eicrecon_shell hello.cpp)
target_link_options(eicrecon_shell PRIVATE -lembind -O3)
target_link_libraries(eicrecon_shell PRIVATE podio::podio)
target_link_libraries(eicrecon_shell PRIVATE EDM4hep::edm4hep)
