#include <iostream>
#include <string>
#include <emscripten/bind.h>

std::string greet(const std::string& name) {
  return "EICrecon WebAssembly Interface initialized for: " + name;
}

EMSCRIPTEN_BINDINGS(eicrecon_wasm) { emscripten::function("greet", &greet); }
