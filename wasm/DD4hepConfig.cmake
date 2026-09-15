# Dummy DD4hepConfig.cmake to satisfy downstream libraries (e.g., algorithms)
# that unconditionally require DD4hep in a WebAssembly build where DD4hep is
# not available or needed.

set(DD4hep_FOUND TRUE)
set(DD4HEP_FOUND TRUE)
set(DD4hep_VERSION "1.0.0")

if(NOT TARGET DD4hep::DDCore)
  add_library(DD4hep::DDCore INTERFACE IMPORTED)
endif()
if(NOT TARGET DD4hep::DDRec)
  add_library(DD4hep::DDRec INTERFACE IMPORTED)
endif()
if(NOT TARGET DD4hep::DDCond)
  add_library(DD4hep::DDCond INTERFACE IMPORTED)
endif()
if(NOT TARGET DD4hep::DDAlign)
  add_library(DD4hep::DDAlign INTERFACE IMPORTED)
endif()
