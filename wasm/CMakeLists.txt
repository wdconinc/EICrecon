cmake_minimum_required(VERSION 3.25)
project(EICrecon_WASM)

# Enforce CMP0077 NEW in all subdirectories (even if they specify older
# cmake_minimum_required)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_INCLUDE_DIR "/usr/include" CACHE PATH "" FORCE)
set(CMAKE_SKIP_INSTALL_RULES ON)

include(FetchContent)

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
FetchContent_Declare(
  ROOT
  GIT_REPOSITORY https://github.com/wdconinc/root.git
  GIT_TAG wasm-system-abi
  OVERRIDE_FIND_PACKAGE)
block()
set(minimal ON)
set(geom ON)
set(wasm ON)
set(cling OFF)
set(builtin_cling OFF)
set(builtin_llvm OFF)
set(builtin_clang OFF)
set(runtime_cxxmodules OFF)
set(gdml OFF)
set(xml OFF)
set(http OFF)
set(ssl OFF)
set(builtin_zlib ON)
set(builtin_lz4 ON)
set(builtin_lzma ON)
set(builtin_zstd ON)
set(builtin_xxhash ON)
set(builtin_pcre ON)
set(builtin_freetype ON)
set(imt OFF)
set(tbb OFF)
set(physics ON)
set(tree ON)
set(BUILD_TESTING OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(ROOT)
endblock()

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
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

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
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

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
FetchContent_Declare(
  podio
  GIT_REPOSITORY https://github.com/wdconinc/podio.git
  GIT_TAG wasm
  OVERRIDE_FIND_PACKAGE)

block()
set(ENABLE_ROOT ON)
set(ENABLE_SIO OFF)
set(BUILD_TESTING OFF)
set(BUILD_TESTS OFF)
set(podio_BUILD_DOXYGEN OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(podio)
endblock()

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
FetchContent_Declare(
  edm4hep
  GIT_REPOSITORY https://github.com/wdconinc/EDM4hep.git
  GIT_TAG fix-dictionary-dependency
  OVERRIDE_FIND_PACKAGE)

block()
set(ENABLE_ROOT ON)
set(BUILD_TESTING OFF)
set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
set(EDM4HEP_WITH_JSON OFF)
FetchContent_MakeAvailable(edm4hep)
endblock()
FetchContent_GetProperties(edm4hep)
set(EDM4HEP_DATA_DIR "${edm4hep_SOURCE_DIR}")

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
FetchContent_Declare(
  edm4eic
  GIT_REPOSITORY https://github.com/eic/edm4eic.git
  GIT_TAG main
  OVERRIDE_FIND_PACKAGE)

block()
set(ENABLE_ROOT ON)
set(BUILD_TESTING OFF)
set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
set(EDM4EIC_WITH_JSON OFF)
FetchContent_MakeAvailable(edm4eic)
endblock()

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
FetchContent_Declare(
  jana2
  GIT_REPOSITORY https://github.com/wdconinc/JANA2.git
  GIT_TAG wasm
  OVERRIDE_FIND_PACKAGE)

block()
set(USE_ROOT ON)
set(USE_PODIO ON)
set(USE_ZEROMQ OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)
set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(jana2)
endblock()

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
FetchContent_Declare(
  DD4hep
  GIT_REPOSITORY https://github.com/wdconinc/DD4hep.git
  GIT_TAG wasm-system-abi
  OVERRIDE_FIND_PACKAGE)
block()
set(DD4HEP_USE_XERCESC OFF)
set(DD4HEP_USE_GEANT4 OFF)
set(DD4HEP_BUILD_EXAMPLES OFF)
set(BUILD_TESTING OFF)
FetchContent_MakeAvailable(DD4hep)
endblock()

set(ROOT_USE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/empty.cmake)
set(ROOT_VERSION "6.32.0")
FetchContent_Declare(
  algorithms
  GIT_REPOSITORY https://github.com/eic/algorithms.git
  GIT_TAG main
  OVERRIDE_FIND_PACKAGE
  PATCH_COMMAND sed -i "s/.*VERSION_LESS 0.4.1.*/if(0)/" CMakeLists.txt && sed -i "s/SHARED//g" core/CMakeLists.txt)

block()
set(BUILD_TESTING OFF)
set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
set(Microsoft.GSL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ---------------------------------------------------------------------------
# Aliases and Dummies for WASM missing pieces
# ---------------------------------------------------------------------------

if(NOT TARGET Microsoft.GSL::GSL AND TARGET GSL)
  add_library(Microsoft.GSL::GSL ALIAS GSL)
endif()
if(NOT TARGET EDM4HEP::edm4hep AND TARGET edm4hep)
  add_library(EDM4HEP::edm4hep ALIAS edm4hep)
endif()
if(NOT TARGET EDM4EIC::edm4eic AND TARGET edm4eic)
  add_library(EDM4EIC::edm4eic ALIAS edm4eic)
endif()

FetchContent_MakeAvailable(algorithms)
endblock()

# ---------------------------------------------------------------------------
# Minimal EICrecon Build
# ---------------------------------------------------------------------------
add_library(minimal_plugin STATIC plugins/MinimalPlugin/MinimalPlugin.cc)
target_include_directories(
  minimal_plugin
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugins/MinimalPlugin
          ${jana2_SOURCE_DIR}/src/libraries ${jana2_BINARY_DIR}/src/libraries)
target_link_libraries(minimal_plugin PUBLIC jana2 EDM4EIC::edm4eic core)

add_executable(eicrecon_wasm eicrecon_main.cc)
target_include_directories(
  eicrecon_wasm PRIVATE ${jana2_SOURCE_DIR}/src/libraries
                        ${jana2_BINARY_DIR}/src/libraries)
target_link_libraries(eicrecon_wasm PUBLIC jana2 minimal_plugin)
set_target_properties(eicrecon_wasm PROPERTIES OUTPUT_NAME eicrecon)
