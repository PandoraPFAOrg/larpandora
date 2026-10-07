//############################################################################
//### Name:        ShowerNumElectronsEnergy                                ###
//### Author:      Tom Ham                                                 ###
//### Date:        01/04/2020                                              ###
//### Description: Tool for finding the Energy of the shower by going      ###
//###              from number of hits -> number of electrons -> energy.   ###
//###              Derived from the linear energy algorithm, written for   ###
//###              the EMShower_module.cc                                  ###
//############################################################################

//Framework Includes
#include "art/Utilities/ToolMacros.h"

//LArSoft Includes
#include "larcore/Geometry/WireReadout.h"
#include "larcoreobj/SimpleTypesAndConstants/PhysicalConstants.h"
#include "lardata/DetectorInfoServices/DetectorClocksService.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "lardataobj/RecoBase/Cluster.h"
#include "lardataobj/RecoBase/PFParticle.h"
#include "larpandora/LArPandoraEventBuilding/LArPandoraShower/Tools/IShowerTool.h"
#include "larreco/Calorimetry/CalorimetryAlg.h"

// For Calorimetry normalization
#include "art/Utilities/make_tool.h"
#include "lardataobj/RecoBase/SpacePoint.h"
#include "larpandora/LArPandoraInterface/LArPandoraHelper.h"
#include "larreco/Calorimetry/INormalizeCharge.h"

//C++ Includes
#include <tuple>

using namespace lar_pandora;

namespace ShowerRecoTools {

  class ShowerNumElectronsEnergy : IShowerTool {

  public:
    ShowerNumElectronsEnergy(const fhicl::ParameterSet& pset);

    //Physics Function. Calculate the shower Energy.
    int CalculateElement(const art::Ptr<recob::PFParticle>& pfparticle,
                         art::Event& Event,
                         reco::shower::ShowerElementHolder& ShowerElementHolder) override;

  private:
    double CalculateEnergy(const art::Event& event,
                           const detinfo::DetectorClocksData& clockData,
                           const detinfo::DetectorPropertiesData& detProp,
                           const std::vector<art::Ptr<recob::Hit>>& hits,
                           const geo::PlaneID::PlaneID_t plane) const;

    // Normalize the hit charge using its space point position and the shower direction
    double NormalizedHitCharge(const art::Event& e,
                               double charge,
                               const art::Ptr<recob::Hit>& hit) const;

    art::InputTag fPFParticleLabel;
    int fVerbose;

    std::string fShowerEnergyOutputLabel;
    std::string fShowerBestPlaneOutputLabel;
    std::string fShowerDirectionInputLabel;

    std::vector<std::unique_ptr<INormalizeCharge>> fNormalizationTools;
    HitsToSpacePoints fHitsToSpacePoints; // Filled per event when fApplyCorrectionsInNorm
    geo::Vector_t fShowerDir;             // Filled per shower when fApplyCorrectionsInNorm

    //Services
    geo::WireReadoutGeom const& fChannelMap = art::ServiceHandle<geo::WireReadout>()->Get();
    calo::CalorimetryAlg fCalorimetryAlg;

    // Declare stuff
    double fRecombinationFactor;
    bool fApplyCorrectionsInNorm;  // Whether to instead apply calorimetry corrections in norm.
    bool fApplyLifetimeCorrection; // Whether to apply MC lifetime correction
  };

  ShowerNumElectronsEnergy::ShowerNumElectronsEnergy(const fhicl::ParameterSet& pset)
    : IShowerTool(pset.get<fhicl::ParameterSet>("BaseTools"))
    , fPFParticleLabel(pset.get<art::InputTag>("PFParticleLabel"))
    , fVerbose(pset.get<int>("Verbose"))
    , fShowerEnergyOutputLabel(pset.get<std::string>("ShowerEnergyOutputLabel"))
    , fShowerBestPlaneOutputLabel(pset.get<std::string>("ShowerBestPlaneOutputLabel"))
    , fShowerDirectionInputLabel(pset.get<std::string>("ShowerDirectionInputLabel"))
    , fCalorimetryAlg(pset.get<fhicl::ParameterSet>("CalorimetryAlg"))
    , fRecombinationFactor(pset.get<double>("RecombinationFactor"))
    , fApplyCorrectionsInNorm(pset.get<bool>("ApplyCorrectionsInNorm"))
    , fApplyLifetimeCorrection(pset.get<bool>("ApplyLifetimeCorrection"))
  {
    if (fApplyCorrectionsInNorm) {
      auto tool_psets = pset.get<std::vector<fhicl::ParameterSet>>("NormTools");

      for (auto const& tool_pset : tool_psets) {
        fNormalizationTools.push_back(art::make_tool<INormalizeCharge>(tool_pset));
      }
    }
  }

  int ShowerNumElectronsEnergy::CalculateElement(const art::Ptr<recob::PFParticle>& pfparticle,
                                                 art::Event& Event,
                                                 reco::shower::ShowerElementHolder& ShowerEleHolder)
  {

    if (fVerbose)
      std::cout
        << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Shower Reco Energy Tool ~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
        << std::endl;

    // Get the assocated pfParicle vertex PFParticles
    auto const pfpHandle = Event.getValidHandle<std::vector<recob::PFParticle>>(fPFParticleLabel);

    //Get the clusters
    auto const clusHandle = Event.getValidHandle<std::vector<recob::Cluster>>(fPFParticleLabel);

    if (fApplyCorrectionsInNorm) {
      // Setup normalization tools, the shower direction and the hit -> space point map
      fShowerDir = {-999, -999, -999};
      if (ShowerEleHolder.GetElement(fShowerDirectionInputLabel, fShowerDir) != 0) {
        mf::LogError("ShowerNumElectronsEnergy")
          << "ShowerDirection not available but normalization requested, skipping energy "
             "calculation";
        return 1;
      }

      for (auto const& nt : fNormalizationTools)
        nt->setup(Event);

      fHitsToSpacePoints.clear();
      SpacePointVector spacePointVector;
      SpacePointsToHits spacePointsToHits;
      LArPandoraHelper::CollectSpacePoints(
        Event, fPFParticleLabel.label(), spacePointVector, spacePointsToHits, fHitsToSpacePoints);
    }

    const art::FindManyP<recob::Cluster>& fmc =
      ShowerEleHolder.GetFindManyP<recob::Cluster>(pfpHandle, Event, fPFParticleLabel);
    // art::FindManyP<recob::Cluster> fmc(pfpHandle, Event, fPFParticleLabel);
    std::vector<art::Ptr<recob::Cluster>> clusters = fmc.at(pfparticle.key());

    //Get the hit association
    const art::FindManyP<recob::Hit>& fmhc =
      ShowerEleHolder.GetFindManyP<recob::Hit>(clusHandle, Event, fPFParticleLabel);
    // art::FindManyP<recob::Hit> fmhc(clusHandle, Event, fPFParticleLabel);

    std::map<geo::PlaneID::PlaneID_t, std::vector<art::Ptr<recob::Hit>>> planeHits;

    //Loop over the clusters in the plane and get the hits
    for (auto const& cluster : clusters) {

      //Get the hits
      std::vector<art::Ptr<recob::Hit>> hits = fmhc.at(cluster.key());

      //Get the plane.
      const geo::PlaneID::PlaneID_t plane(cluster->Plane().Plane);

      planeHits[plane].insert(planeHits[plane].end(), hits.begin(), hits.end());
    }

    // Calculate the energy for each plane && best plane
    geo::PlaneID::PlaneID_t bestPlane = std::numeric_limits<geo::PlaneID::PlaneID_t>::max();
    unsigned int bestPlaneNumHits = 0;

    //Holder for the final product
    std::vector<double> energyVec(fChannelMap.Nplanes(), -999.);
    std::vector<double> energyError(fChannelMap.Nplanes(), -999.);

    auto const clockData =
      art::ServiceHandle<detinfo::DetectorClocksService const>()->DataFor(Event);
    auto const detProp =
      art::ServiceHandle<detinfo::DetectorPropertiesService const>()->DataFor(Event, clockData);

    for (auto const& [plane, hits] : planeHits) {

      unsigned int planeNumHits = hits.size();

      //Calculate the Energy for
      double energy = CalculateEnergy(Event, clockData, detProp, hits, plane);

      // If the energy is negative, leave it at -999
      if (energy > 0) energyVec.at(plane) = energy;

      if (planeNumHits > bestPlaneNumHits) {
        bestPlane = plane;
        bestPlaneNumHits = planeNumHits;
      }
    }

    ShowerEleHolder.SetElement(energyVec, energyError, fShowerEnergyOutputLabel);
    // Only set the best plane if it has some hits in it
    if (bestPlane < fChannelMap.Nplanes()) {
      // Need to cast as an int for legacy default of -999
      // have to define a new variable as we pass-by-reference when filling
      int bestPlaneVal(bestPlane);
      ShowerEleHolder.SetElement(bestPlaneVal, fShowerBestPlaneOutputLabel);
    }

    return 0;
  }

  // function to calculate the reco energy
  double ShowerNumElectronsEnergy::CalculateEnergy(const art::Event& event,
                                                   const detinfo::DetectorClocksData& clockData,
                                                   const detinfo::DetectorPropertiesData& detProp,
                                                   const std::vector<art::Ptr<recob::Hit>>& hits,
                                                   const geo::PlaneID::PlaneID_t plane) const
  {

    if (fApplyCorrectionsInNorm && fHitsToSpacePoints.empty()) {
      if (fVerbose) {
        mf::LogError("ShowerNumElectronsEnergy")
          << "No hits to space points mapping provided while requesting normalization, returning "
             "error energy value -999"
          << std::endl;
      }
      return -999;
    }

    double totalEnergy = 0;
    double correctedtotalCharge = 0;
    double nElectrons = 0;

    for (auto const& hit : hits) {

      double hitCharge = fApplyLifetimeCorrection ?
                           hit->Integral() * fCalorimetryAlg.LifetimeCorrection(
                                               clockData, detProp, hit->PeakTime()) :
                           hit->Integral();

      hitCharge /= fRecombinationFactor;

      if (fApplyCorrectionsInNorm) hitCharge = NormalizedHitCharge(event, hitCharge, hit);

      correctedtotalCharge += hitCharge;
    }
    // calculate # of electrons and the corresponding energy
    nElectrons = fCalorimetryAlg.ElectronsFromADCArea(correctedtotalCharge, plane);
    totalEnergy = (nElectrons / util::kGeVToElectrons) * 1000; // energy in MeV
    return totalEnergy;
  }

  double ShowerNumElectronsEnergy::NormalizedHitCharge(const art::Event& e,
                                                       double charge,
                                                       const art::Ptr<recob::Hit>& hit) const
  {
    // Hits without space points contribute uncorrected charge
    auto const hIter = fHitsToSpacePoints.find(hit);
    if (hIter == fHitsToSpacePoints.end()) return charge;

    double ret = charge;
    for (auto const& nt : fNormalizationTools)
      ret = nt->Normalize(ret, e, *hit, hIter->second->position(), fShowerDir, 0);

    return ret;
  }
}

DEFINE_ART_CLASS_TOOL(ShowerRecoTools::ShowerNumElectronsEnergy)
