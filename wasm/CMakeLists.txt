cmake_minimum_required(VERSION 3.25)
project(EICrecon_WASM)

# Enforce CMP0077 NEW in all subdirectories (even if they specify older
# cmake_minimum_required)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
  OVERRIDE_FIND_PACKAGE)
block()
set(FMT_DOC OFF)
set(FMT_TEST OFF)
FetchContent_MakeAvailable(fmt)
endblock()

FetchContent_Declare(
  podio
  GIT_REPOSITORY https://github.com/wdconinc/podio.git
  GIT_TAG wasm
  OVERRIDE_FIND_PACKAGE)

block()
set(ENABLE_ROOT OFF)
set(ENABLE_SIO OFF)
set(BUILD_TESTING OFF)
set(podio_BUILD_DOXYGEN OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(podio)
endblock()

FetchContent_Declare(
  edm4hep
  GIT_REPOSITORY https://github.com/wdconinc/EDM4hep.git
  GIT_TAG fix-dictionary-dependency
  OVERRIDE_FIND_PACKAGE)

block()
set(ENABLE_ROOT OFF)
set(BUILD_TESTING OFF)
set(BUILD_SHARED_LIBS OFF)
set(EDM4HEP_WITH_JSON OFF)
FetchContent_MakeAvailable(edm4hep)
endblock()
FetchContent_GetProperties(edm4hep)
set(EDM4HEP_DATA_DIR "${edm4hep_SOURCE_DIR}")

FetchContent_Declare(
  edm4eic
  GIT_REPOSITORY https://github.com/eic/edm4eic.git
  GIT_TAG main
  OVERRIDE_FIND_PACKAGE)

block()
set(ENABLE_ROOT OFF)
set(BUILD_TESTING OFF)
set(BUILD_SHARED_LIBS OFF)
set(EDM4EIC_WITH_JSON OFF)
FetchContent_MakeAvailable(edm4eic)
endblock()

FetchContent_Declare(
  jana2
  GIT_REPOSITORY https://github.com/wdconinc/JANA2.git
  GIT_TAG wasm
  OVERRIDE_FIND_PACKAGE)

block()
set(USE_ROOT OFF)
set(USE_PODIO OFF)
set(USE_ZEROMQ OFF)
set(BUILD_TESTING OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(jana2)
endblock()

add_executable(eicrecon_shell hello.cpp)
target_link_options(eicrecon_shell PRIVATE -lembind -O3)
target_link_libraries(eicrecon_shell PRIVATE podio::podio)
target_link_libraries(eicrecon_shell PRIVATE edm4hep)
target_link_libraries(eicrecon_shell PRIVATE edm4eic)
target_link_libraries(eicrecon_shell PRIVATE jana2_static_lib)
target_include_directories(eicrecon_shell
                           PRIVATE ${jana2_SOURCE_DIR}/src/libraries)
