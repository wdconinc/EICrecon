#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <emscripten/bind.h>
#include <podio/podioVersion.h>
#include <edm4hep/MCParticle.h>

std::string execute_command(const std::string& cmd_line) {
  std::istringstream iss(cmd_line);
  std::vector<std::string> args;
  std::string token;
  while (iss >> token) {
    args.push_back(token);
  }

  if (args.empty())
    return "";

  std::ostringstream out;
  std::string cmd = args[0];

  if (cmd == "help") {
    out << "Available commands:\n";
    out << "  eicrecon   - Run the mock EICrecon framework\n";
    out << "  echo       - Print arguments\n";
    out << "  podio      - Show Podio version info\n";
    out << "  edm4hep    - Test EDM4hep integration\n";
    out << "  clear      - Clear the terminal\n";
    out << "  help       - Show this message";
  } else if (cmd == "echo") {
    for (size_t i = 1; i < args.size(); ++i) {
      out << args[i] << (i + 1 == args.size() ? "" : " ");
    }
  } else if (cmd == "edm4hep") {
    edm4hep::MCParticle particle;
    out << "EDM4hep built successfully!\n";
    out << "Created an MCParticle. Is available? " << (particle.isAvailable() ? "Yes" : "No")
        << "\n";
  } else if (cmd == "podio") {
    out << "Podio built successfully!\n";
    out << "Podio version: " << podio::version::build_version << "\n";
  } else if (cmd == "eicrecon") {
    out << "eicrecon (WASM build)\n";
    out << "Initializing JANA2 framework...\n";
    for (size_t i = 1; i < args.size(); ++i) {
      out << "Applying argument: " << args[i] << "\n";
    }
    out << "Processing events...\n";
    out << "Loaded 0 plugins.\n";
    out << "Done.";
  } else {
    out << "bash: " << cmd << ": command not found\nType 'help' for available commands.";
  }
  return out.str();
}

EMSCRIPTEN_BINDINGS(eicrecon_wasm) { emscripten::function("execute_command", &execute_command); }
