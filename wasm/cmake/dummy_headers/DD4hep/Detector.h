#pragma once
#include <memory>
#include <string>
namespace dd4hep {
class DetElement {};
class Detector {
public:
  static std::unique_ptr<Detector> make_unique(const std::string&) {
    return std::make_unique<Detector>();
  }
  void fromCompact(const std::string&) {}
  void volumeManager() {}
  void apply(const char*, int, void*) {}
  DetElement world() const { return DetElement(); }
};
} // namespace dd4hep
