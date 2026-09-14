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
  microsoft_gsl
  GIT_REPOSITORY https://github.com/microsoft/GSL.git
  GIT_TAG v4.0.0
  OVERRIDE_FIND_PACKAGE)
block()
set(GSL_TEST OFF)
set(GSL_INSTALL ON)
FetchContent_MakeAvailable(microsoft_gsl)
endblock()
# Provide the alias expected by downstream
if(NOT TARGET Microsoft.GSL::GSL)
  add_library(Microsoft.GSL::GSL ALIAS GSL)
endif()

FetchContent_Declare(
  podio
  GIT_REPOSITORY https://github.com/wdconinc/podio.git
  GIT_TAG wasm
  OVERRIDE_FIND_PACKAGE)

block()
set(ENABLE_ROOT OFF)
set(ENABLE_SIO OFF)
set(BUILD_TESTING OFF)
set(BUILD_TESTS OFF)
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
set(BUILD_TESTS OFF)
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
set(BUILD_TESTS OFF)
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
set(USE_PODIO ON)
set(USE_ZEROMQ OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)
set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(jana2)
endblock()

FetchContent_Declare(
  algorithms
  GIT_REPOSITORY https://github.com/eic/algorithms.git
  GIT_TAG main
  OVERRIDE_FIND_PACKAGE
  PATCH_COMMAND sed -i "s/.*VERSION_LESS 0.4.1.*/if(0)/" CMakeLists.txt)

block()
set(BUILD_TESTING OFF)
set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
set(Microsoft.GSL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(DD4hep_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ---------------------------------------------------------------------------
# Aliases and Dummies for WASM missing pieces
# ---------------------------------------------------------------------------
if(NOT TARGET EDM4HEP::edm4hep AND TARGET edm4hep)
  add_library(EDM4HEP::edm4hep ALIAS edm4hep)
endif()
if(NOT TARGET EDM4EIC::edm4eic AND TARGET edm4eic)
  add_library(EDM4EIC::edm4eic ALIAS edm4eic)
endif()
if(NOT TARGET podio::podioRootIO)
  add_library(podioRootIO INTERFACE)
  add_library(podio::podioRootIO ALIAS podioRootIO)
endif()
if(NOT TARGET podio::podioDict)
  add_library(podioDict INTERFACE)
  add_library(podio::podioDict ALIAS podioDict)
endif()

FetchContent_MakeAvailable(algorithms)
endblock()

# ---------------------------------------------------------------------------
# Minimal EICrecon Build
# ---------------------------------------------------------------------------
add_library(minimal_plugin STATIC plugins/MinimalPlugin/MinimalPlugin.cc)
target_include_directories(
  minimal_plugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugins/MinimalPlugin)
target_link_libraries(minimal_plugin PUBLIC jana2 EDM4EIC::edm4eic core)

add_executable(eicrecon_wasm eicrecon_main.cc)
target_link_libraries(eicrecon_wasm PUBLIC jana2 minimal_plugin)
set_target_properties(eicrecon_wasm PROPERTIES OUTPUT_NAME eicrecon)
