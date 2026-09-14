#pragma once
namespace ROOT {
namespace Math {
  class XYZVector {
  public:
    XYZVector(double, double, double) {}
    double Eta() const { return 0; }
    double Theta() const { return 0; }
  };
} // namespace Math
} // namespace ROOT
