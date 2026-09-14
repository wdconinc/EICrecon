// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EIC Collaboration
//
// Minimal EICrecon plugin for WebAssembly demo.
// Applies the Cloner algorithm to MCParticles.

#include <JANA/JApplication.h>
#include <JANA/JEventProcessor.h>
#include <JANA/Components/JHasInputs.h>

#include <algorithms/algorithm.h>
#include <edm4hep/MCParticleCollection.h>

#include <fmt/format.h>
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// Minimal stand-alone Cloner (no spdlog / Log_service dependency)
// ---------------------------------------------------------------------------
namespace eicrecon {

template <typename T>
class Cloner : public algorithms::Algorithm<algorithms::Input<typename T::collection_type>,
                                            algorithms::Output<typename T::collection_type>> {
public:
  explicit Cloner(std::string name)
      : algorithms::Algorithm<algorithms::Input<typename T::collection_type>,
                              algorithms::Output<typename T::collection_type>>(
            std::move(name), {"inputCollection"}, {"outputCollection"},
            "Clone collection elements to a standalone collection") {}

  void process(const typename Cloner::Input& input,
               const typename Cloner::Output& output) const final {
    const auto [in_coll] = input;
    auto [out_coll]      = output;
    for (const auto& obj : *in_coll) {
      out_coll->push_back(obj.clone());
    }
  }
};

} // namespace eicrecon

// ---------------------------------------------------------------------------
// JEventProcessor
// ---------------------------------------------------------------------------
class MinimalProcessor : public JEventProcessor {
public:
  MinimalProcessor() {
    SetTypeName("MinimalProcessor");
    // Declare an output collection produced by the Cloner algorithm
    m_input_tag = "MCParticles";
  }

  void Init() override { std::cout << "[MinimalPlugin] Init: will clone MCParticle collection\n"; }

  void Process(const std::shared_ptr<const JEvent>& event) override {
    // Retrieve MCParticles
    auto* mcparts = event->GetCollection<edm4hep::MCParticle>(m_input_tag);
    if (!mcparts) {
      return;
    }
    std::cout << fmt::format("[MinimalPlugin] Event {}: {} MCParticles\n", event->GetEventNumber(),
                             mcparts->size());
  }

  void Finish() override { std::cout << "[MinimalPlugin] Finish\n"; }

private:
  std::string m_input_tag;
};

// ---------------------------------------------------------------------------
// Plugin entry point
// ---------------------------------------------------------------------------
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->Add(new MinimalProcessor());
}
}
