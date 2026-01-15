// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov, Shujie Li, Dmitry Kalinkin

#include "CKFTracking.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#if Acts_VERSION_MAJOR >= 46
#include <Acts/EventData/BoundTrackParameters.hpp>
#else
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#endif
#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryHierarchyMap.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilterExtensions.hpp>
#include <Acts/Utilities/CalibrationContext.hpp>
#include <spdlog/common.h>
#include <algorithm>
#include <any>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/ProxyAccessor.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Propagator/ActorList.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/MaterialInteractor.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/PropagatorOptions.hpp>
#include <Acts/Propagator/StandardAborters.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/TrackFinding/TrackStateCreator.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/TrackHelpers.hpp>
#include <ActsExamples/EventData/GeometryContainers.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov3f.h>
#include <edm4eic/Cov6f.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU> // IWYU pragma: keep
// IWYU pragma: no_include <Acts/Utilities/detail/ContextType.hpp>
// IWYU pragma: no_include <Acts/Utilities/detail/ContainerIterator.hpp>

#include "ActsGeometryProvider.h"
#include "PodioMeasurementCalibration.h"
#include "extensions/edm4eic/EDM4eicToActs.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep
#include "extensions/spdlog/SpdlogToActs.h"

namespace {

/// Calibrator that reads directly from edm4eic::Measurement2DCollection and works with Podio backend.
/// Also owns the geometry-ordered IndexSourceLink multiset built from the same collection.
class EDM4eicMeasurementSourceLinkCalibrator {
public:
  explicit EDM4eicMeasurementSourceLinkCalibrator(const edm4eic::Measurement2DCollection* meas2Ds)
      : m_meas2Ds(meas2Ds) {
    for (std::size_t index = 0; index < meas2Ds->size(); ++index) {
      m_orderedSourceLinks.emplace(Acts::GeometryIdentifier{(*meas2Ds)[index].getSurface()}, index);
    }
  }

  const ActsExamples::GeometryIdMultiset<ActsExamples::IndexSourceLink>&
  orderedSourceLinks() const {
    return m_orderedSourceLinks;
  }

  void calibrate(const Acts::GeometryContext& /*gctx*/, const Acts::CalibrationContext& /*cctx*/,
                 const Acts::SourceLink& sourceLink,
                 ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>::TrackStateProxy
                     trackState) const {
    trackState.setUncalibratedSourceLink(Acts::SourceLink{sourceLink});
    const auto& idxSourceLink = sourceLink.get<ActsExamples::IndexSourceLink>();
    const auto& meas2D        = (*m_meas2Ds)[idxSourceLink.index()];

#if Acts_VERSION_MAJOR > 45 || (Acts_VERSION_MAJOR == 45 && Acts_VERSION_MINOR >= 2)
    Acts::Vector<2> loc       = Acts::Vector2::Zero();
    Acts::SquareMatrix<2> cov = Acts::SquareMatrix<2>::Zero();
#else
    Acts::ActsVector<2> loc       = Acts::Vector2::Zero();
    Acts::ActsSquareMatrix<2> cov = Acts::ActsSquareMatrix<2>::Zero();
#endif
    constexpr auto mm                       = Acts::UnitConstants::mm / edm4eic::unit::mm;
    constexpr auto mm2                      = mm * mm;
    loc[Acts::eBoundLoc0]                   = meas2D.getLoc().a * mm;
    loc[Acts::eBoundLoc1]                   = meas2D.getLoc().b * mm;
    cov(Acts::eBoundLoc0, Acts::eBoundLoc0) = meas2D.getCovariance().xx * mm2;
    cov(Acts::eBoundLoc1, Acts::eBoundLoc1) = meas2D.getCovariance().yy * mm2;
    cov(Acts::eBoundLoc0, Acts::eBoundLoc1) = meas2D.getCovariance().xy * mm2;
    cov(Acts::eBoundLoc1, Acts::eBoundLoc0) = meas2D.getCovariance().xy * mm2;

    trackState.allocateCalibrated(loc, cov);
    std::array<uint8_t, 2> indices{static_cast<uint8_t>(Acts::eBoundLoc0),
                                   static_cast<uint8_t>(Acts::eBoundLoc1)};
    trackState.setProjectorSubspaceIndices(indices);
  }

private:
  const edm4eic::Measurement2DCollection* m_meas2Ds;
  ActsExamples::GeometryIdMultiset<ActsExamples::IndexSourceLink> m_orderedSourceLinks;
};

} // anonymous namespace

namespace eicrecon {

using namespace Acts::UnitLiterals;

void CKFTracking::init() {
  m_acts_logger = Acts::getDefaultLogger(
      "CKF", eicrecon::SpdlogToActsLevel(static_cast<spdlog::level::level_enum>(this->level())));

  // eta bins, chi2 and #sourclinks per surface cutoffs
  m_sourcelinkSelectorCfg = {
      {Acts::GeometryIdentifier(),
       {.etaBins               = m_cfg.etaBins,
        .chi2CutOff            = m_cfg.chi2CutOff,
        .numMeasurementsCutOff = {m_cfg.numMeasurementsCutOff.begin(),
                                  m_cfg.numMeasurementsCutOff.end()}}},
  };
  m_trackFinderFunc = CKFTracking::makeCKFTrackingFunction(
      m_geoSvc->trackingGeometry(), m_geoSvc->getFieldProvider(), acts_logger());
}

void CKFTracking::process(const Input& input, const Output& output) const {
  const auto [init_trk_seeds, meas2Ds] = input;
  auto [output_track_states, output_track_parameters, output_track_jacobians, output_tracks] =
      output;

  // If measurements or initial track parameters are empty, return early
  if (meas2Ds->empty() || init_trk_seeds->empty()) {
    return;
  }

  ActsExamples::TrackParametersContainer acts_init_trk_params;
  for (const auto& track_seed : *init_trk_seeds) {

    const auto& track_parameter = track_seed.getParams();

    Acts::BoundVector params;
    params(Acts::eBoundLoc0) =
        track_parameter.getLoc().a * Acts::UnitConstants::mm; // cylinder radius
    params(Acts::eBoundLoc1) =
        track_parameter.getLoc().b * Acts::UnitConstants::mm; // cylinder length
    params(Acts::eBoundPhi)    = track_parameter.getPhi();
    params(Acts::eBoundTheta)  = track_parameter.getTheta();
    params(Acts::eBoundQOverP) = track_parameter.getQOverP() / Acts::UnitConstants::GeV;
    params(Acts::eBoundTime)   = track_parameter.getTime() * Acts::UnitConstants::ns;

#if Acts_VERSION_MAJOR > 45 || (Acts_VERSION_MAJOR == 45 && Acts_VERSION_MINOR >= 1)
    Acts::BoundMatrix cov = Acts::BoundMatrix::Zero();
#else
    Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
#endif
    for (std::size_t i = 0; const auto& [a, x] : edm4eic_indexed_units) {
      for (std::size_t j = 0; const auto& [b, y] : edm4eic_indexed_units) {
        cov(a, b) = track_parameter.getCovariance()(i, j) * x * y;
        ++j;
      }
      ++i;
    }

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(Acts::Vector3(0, 0, 0));

    // Create parameters
    acts_init_trk_params.emplace_back(pSurface, params, cov, Acts::ParticleHypothesis::pion());
  }

  //// Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

  // Convert algorithm log level to Acts log level for local logger
  const auto spdlog_level = static_cast<spdlog::level::level_enum>(this->level());
  const auto acts_level   = eicrecon::SpdlogToActsLevel(spdlog_level);
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("CKF", acts_level));

  // Get run-scoped contexts from service
  const auto& gctx = m_geoSvc->getActsGeometryContext();
  const auto& mctx = m_geoSvc->getActsMagneticFieldContext();
  const auto& cctx = m_geoSvc->getActsCalibrationContext();

  // Create conversion helper for Podio backend
  PodioGeometryIdConversionHelper helper(gctx, m_geoSvc->trackingGeometry());

  // Create track container with ActsPodioEdm backend using externally owned collections
  auto trackStateContainer =
      std::make_shared<ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>>(
          helper, Acts::RefHolder{*output_track_states}, Acts::RefHolder{*output_track_parameters},
          Acts::RefHolder{*output_track_jacobians});
  auto trackContainer = std::make_shared<ActsPlugins::MutablePodioTrackContainer<Acts::RefHolder>>(
      helper, Acts::RefHolder{*output_tracks});

  // Create Acts TrackContainer with Podio backends using shared_ptr holder
  PodioTrackContainer acts_tracks(trackContainer, trackStateContainer);

  Acts::PropagatorPlainOptions pOptions(gctx, mctx);
  pOptions.maxSteps = 10000;

  EDM4eicMeasurementSourceLinkCalibrator calibratorImpl{meas2Ds};
  Acts::GainMatrixUpdater kfUpdater;
  Acts::MeasurementSelector measSel{m_sourcelinkSelectorCfg};

  Acts::CombinatorialKalmanFilterExtensions<PodioTrackContainer> extensions;
  extensions.updater.connect<&Acts::GainMatrixUpdater::operator()<
      typename PodioTrackContainer::TrackStateContainerBackend>>(&kfUpdater);

  ActsExamples::IndexSourceLinkAccessor slAccessor;
  slAccessor.container = &calibratorImpl.orderedSourceLinks();
  using TrackStateCreatorType =
      Acts::TrackStateCreator<ActsExamples::IndexSourceLinkAccessor::Iterator, PodioTrackContainer>;
  TrackStateCreatorType trackStateCreator;
  trackStateCreator.sourceLinkAccessor
      .template connect<&ActsExamples::IndexSourceLinkAccessor::range>(&slAccessor);
  trackStateCreator.calibrator.template connect<&EDM4eicMeasurementSourceLinkCalibrator::calibrate>(
      &calibratorImpl);
  trackStateCreator.measurementSelector.template connect<&Acts::MeasurementSelector::select<
      ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>>>(&measSel);

  extensions.createTrackStates.template connect<&TrackStateCreatorType::createTrackStates>(
      &trackStateCreator);

  // Set the CombinatorialKalmanFilter options
  CKFTracking::TrackFinderOptions options(gctx, mctx, cctx, extensions, pOptions);

  using Extrapolator        = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
  using ExtrapolatorOptions = Extrapolator::template Options<
      Acts::ActorList<Acts::MaterialInteractor, Acts::EndOfWorldReached>>;
  Extrapolator extrapolator(Acts::EigenStepper<>(m_BField),
                            Acts::Navigator({.trackingGeometry = m_geoSvc->trackingGeometry()},
                                            acts_logger().cloneWithSuffix("Navigator")),
                            acts_logger().cloneWithSuffix("Propagator"));
  ExtrapolatorOptions extrapolationOptions(gctx, mctx);

  // Create temporary track containers for intermediate processing
  // Heap-allocate collections to ensure proper lifetime management across all copyFrom operations
  auto tempTrackStates = std::make_shared<ActsPodioEdm::TrackStateCollection>();
  auto tempTrackParams = std::make_shared<ActsPodioEdm::BoundParametersCollection>();
  auto tempTrackJacs   = std::make_shared<ActsPodioEdm::JacobianCollection>();
  auto tempTracks      = std::make_shared<ActsPodioEdm::TrackCollection>();

  auto trackStateContainerTemp =
      std::make_shared<ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>>(
          helper, Acts::RefHolder<ActsPodioEdm::TrackStateCollection>{*tempTrackStates},
          Acts::RefHolder<ActsPodioEdm::BoundParametersCollection>{*tempTrackParams},
          Acts::RefHolder<ActsPodioEdm::JacobianCollection>{*tempTrackJacs});
  auto trackContainerTemp =
      std::make_shared<ActsPlugins::MutablePodioTrackContainer<Acts::RefHolder>>(
          helper, Acts::RefHolder<ActsPodioEdm::TrackCollection>{*tempTracks});
  PodioTrackContainer acts_tracks_temp(trackContainerTemp, trackStateContainerTemp);

  // TODO: Add seed number tracking back later using PODIO links or associations
  // Dynamic columns in PODIO backend currently cause serialization issues
  // acts_tracks.addColumn<unsigned int>("seed");
  // acts_tracks_temp.addColumn<unsigned int>("seed");
  // Acts::ProxyAccessor<unsigned int> seedNumber("seed");

  // Loop over seeds
  for (std::size_t iseed = 0; iseed < acts_init_trk_params.size(); ++iseed) {

    // Clear trackContainerTemp and trackStateContainerTemp
    acts_tracks_temp.clear();

    // Run track finding for this seed
    auto result = (*m_trackFinderFunc)(acts_init_trk_params.at(iseed), options, acts_tracks_temp);

    if (!result.ok()) {
      debug("Track finding failed for seed {} with error {}", iseed, result.error().message());
      continue;
    }

    // Set seed number for all found tracks
    auto& tracksForSeed = result.value();
    for (auto& track : tracksForSeed) {
      // Check if track has at least one valid (non-outlier) measurement
      // (this check avoids errors inside smoothing and extrapolation)
      auto lastMeasurement = Acts::findLastMeasurementState(track);
      if (!lastMeasurement.ok()) {
        debug("Track {} for seed {} has no valid measurements, skipping", track.index(), iseed);
        continue;
      }

      if (track.nMeasurements() < m_cfg.numMeasurementsMin) {
        trace("Track {} for seed {} has fewer measurements than minimum of {}, skipping",
              track.index(), iseed, m_cfg.numMeasurementsMin);
        continue;
      }

      auto smoothingResult = Acts::smoothTrack(gctx, track, acts_logger());
      if (!smoothingResult.ok()) {
        debug("Smoothing for seed {} and track {} failed with error {}", iseed, track.index(),
              smoothingResult.error().message());
        continue;
      }

      auto extrapolationResult = Acts::extrapolateTrackToReferenceSurface(
          track, *pSurface, extrapolator, extrapolationOptions,
          Acts::TrackExtrapolationStrategy::firstOrLast, acts_logger());

      if (!extrapolationResult.ok()) {
        debug("Extrapolation for seed {} and track {} failed with error {}", iseed, track.index(),
              extrapolationResult.error().message());
        continue;
      }

      // TODO: Restore seed number tracking when dynamic columns are fixed
      // seedNumber(track) = iseed;

      // Copy accepted track into main track container
      auto acts_tracks_proxy = acts_tracks.makeTrack();
      acts_tracks_proxy.copyFrom(track);

      // Set the reference surface on the output track
      acts_tracks_proxy.setReferenceSurface(pSurface);
    }
  }
}

} // namespace eicrecon
