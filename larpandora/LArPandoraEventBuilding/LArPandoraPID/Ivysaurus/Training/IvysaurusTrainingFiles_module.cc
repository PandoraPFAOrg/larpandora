/**
 *  @file   larpandora/LArPandoraEventBuilding/LArPandoraPID/Ivysaurus/Training/IvysaurusTrainingFiles_module.cc
 *
 *  @brief  Module to create the training files for the ivysaurus PID
 */
// ART
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
// LArSoft
#include "nusimdata/SimulationBase/MCParticle.h"
#include "larpandora/LArPandoraEventBuilding/LArPandoraPID/Ivysaurus/Managers/GridManager.h"
#include "larpandora/LArPandoraEventBuilding/LArPandoraPID/Ivysaurus/Managers/PFPVarManager.h"
#include "larpandora/LArPandoraEventBuilding/LArPandoraPID/Ivysaurus/Managers/TrackVarManager.h"
#include "larpandora/LArPandoraEventBuilding/LArPandoraPID/Ivysaurus/Managers/ShowerVarManager.h"
#include "larpandora/LArPandoraInterface/LArPandoraHelper.h"
// ROOT
#include "TTree.h"

namespace ivysaurus
{

/**
 *  @brief  IvysaurusTrainingFiles class
 */
class IvysaurusTrainingFiles : public art::EDAnalyzer
{
public:
   IvysaurusTrainingFiles(fhicl::ParameterSet const &pset);
   virtual ~IvysaurusTrainingFiles();

   void beginJob();
   void endJob();
   void analyze(const art::Event &evt);
   void Reset();

private:
  // Trees
  TTree *m_tree;
  // Tree variables
  int m_run;
  int m_subrun;
  int m_event;
  // General PFP
  int m_truePDG;
  float m_completeness;
  float m_purity;
  int m_nSpacePoints;
  float m_trueEndX;
  float m_trueEndY;
  float m_trueEndZ;    
  float m_recoEndX;
  float m_recoEndY;
  float m_recoEndZ;
  bool m_isPrimary;
  bool m_isDeltaRay;
  bool m_isMichel;    
  // Plotting
  std::vector<std::vector<double>> m_spacePoints;
  std::vector<std::vector<double>> m_projectionsU;
  std::vector<std::vector<double>> m_projectionsV;
  std::vector<std::vector<double>> m_projectionsW;
  std::vector<float> m_startDriftBoundariesU;
  std::vector<float> m_startDriftBoundariesV;
  std::vector<float> m_startDriftBoundariesW;
  std::vector<float> m_endDriftBoundariesU;
  std::vector<float> m_endDriftBoundariesV;
  std::vector<float> m_endDriftBoundariesW;
  std::vector<float> m_startWireBoundariesU;
  std::vector<float> m_startWireBoundariesV;
  std::vector<float> m_startWireBoundariesW;
  std::vector<float> m_endWireBoundariesU;
  std::vector<float> m_endWireBoundariesV;
  std::vector<float> m_endWireBoundariesW;
  float m_startGridValuesU[576];
  float m_startGridValuesV[576];
  float m_startGridValuesW[576];
  float m_endGridValuesU[576];
  float m_endGridValuesV[576];
  float m_endGridValuesW[576];    
  //PFPVars
  float m_pfpN2DHits;
  float m_pfpTrackShowerScore;
  // TrackVars
  int m_trackVarsSuccessful;
  float m_nTrackChildren;
  float m_nShowerChildren;
  float m_nGrandChildren;
  float m_nChildHits;
  float m_childEnergy;
  float m_childTrackScore;
  float m_trackLength;
  float m_trackWobble;
  float m_trackMomComparison;
  float m_trackDistFromEdge;
  // ShowerVars
  int m_showerVarsSuccessful;
  float m_showerDisplacement;
  float m_DCA;
  float m_trackStubLength;
  float m_fromParentAvSep;
  float m_fromParentChargeAsym;
  // Managers
  GridManager m_gridManager;
  PFPVarManager m_pfpVarManager;
  TrackVarManager m_trackVarManager;
  ShowerVarManager m_showerVarManager;
  // FCL module labels
  std::string m_largeantModuleLabel;    
  std::string m_hitModuleLabel;
  std::string m_recoModuleLabel;
  std::string m_trackModuleLabel;
  std::string m_showerModuleLabel;    
  // Module variables
  float m_completenessThreshold;
  float m_purityThreshold;
  int m_candidateDRMinHits;
  float m_candidateDRMaxTrackScore;
  bool m_writeVisualisationInfo;
};

DEFINE_ART_MODULE(IvysaurusTrainingFiles)

} // namespace ivysaurus

//------------------------------------------------------------------------------------------------------------------------------------------
// ART
#include "art/Framework/Principal/Event.h"
#include "art_root_io/TFileService.h"
#include "art_root_io/TFileDirectory.h"
#include "fhiclcpp/ParameterSet.h"
// LArSoft
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "larpandora/LArPandoraEventBuilding/LArPandoraPID/Ivysaurus/Utils/IvysaurusUtils.h"
#include "larpandora/LArPandoraUtils/PandoraEventUtils.h"
#include "larpandora/LArPandoraUtils/PandoraHitUtils.h"
#include "larpandora/LArPandoraUtils/PandoraPFParticleUtils.h"
#include "larsim/MCCheater/ParticleInventoryService.h"
#include "larsim/Utils/TruthMatchUtils.h"

namespace ivysaurus
{

IvysaurusTrainingFiles::IvysaurusTrainingFiles(fhicl::ParameterSet const &pset) : 
    art::EDAnalyzer(pset),
    m_gridManager(pset.get<fhicl::ParameterSet>("GridManager")),
    m_pfpVarManager(pset.get<fhicl::ParameterSet>("PFPVarManager")),
    m_trackVarManager(pset.get<fhicl::ParameterSet>("TrackVarManager")),
    m_showerVarManager(pset.get<fhicl::ParameterSet>("ShowerVarManager")),
    m_largeantModuleLabel(pset.get<std::string>("LArGeantModuleLabel")),    
    m_hitModuleLabel(pset.get<std::string>("HitModuleLabel")),
    m_recoModuleLabel(pset.get<std::string>("RecoModuleLabel")),    
    m_trackModuleLabel(pset.get<std::string>("TrackModuleLabel")),
    m_showerModuleLabel(pset.get<std::string>("ShowerModuleLabel")),      
    m_completenessThreshold(pset.get<float>("CompletenessThreshold")),
    m_purityThreshold(pset.get<float>("PurityThreshold")),
    m_candidateDRMinHits(pset.get<int>("CandidateDRMinHits")),
    m_candidateDRMaxTrackScore(pset.get<float>("CandidateDRMaxTrackScore")),    
    m_writeVisualisationInfo(pset.get<bool>("WriteVisualisationInfo"))
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

IvysaurusTrainingFiles::~IvysaurusTrainingFiles()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

void IvysaurusTrainingFiles::analyze(const art::Event &evt)
{
    const std::vector<art::Ptr<recob::PFParticle>> pfparticles = lar_pandora::PandoraEventUtils::GetPFParticles(evt, m_recoModuleLabel);

    // Get the neutrino PFP
    if (!lar_pandora::PandoraEventUtils::HasNeutrino(evt, m_recoModuleLabel))
        return;

    art::Ptr<recob::PFParticle> nuPFP = lar_pandora::PandoraEventUtils::GetNeutrino(evt, m_recoModuleLabel);    
    const std::vector<art::Ptr<recob::PFParticle>> &nuChildPFPs = lar_pandora::PandoraPFParticleUtils::GetChildParticles(nuPFP, evt, m_recoModuleLabel);    

    // Prepare for truth matching
    art::ServiceHandle<cheat::ParticleInventoryService> piServ;
    std::map<int, int> rollUpMap;
    const std::vector<art::Ptr<simb::MCParticle>> mcParticles = lar_pandora::PandoraEventUtils::GetMCParticles(evt, m_largeantModuleLabel);
    IvysaurusUtils::PopulateRollUpMap(mcParticles, rollUpMap);

    for (const art::Ptr<recob::PFParticle> &pfparticle : pfparticles)
    {
        if (!lar_pandora::PandoraPFParticleUtils::HasTrack(pfparticle, evt, m_recoModuleLabel, m_trackModuleLabel) &&
            !lar_pandora::PandoraPFParticleUtils::HasShower(pfparticle, evt, m_recoModuleLabel, m_showerModuleLabel))
        {
            continue;
        }
        if (!lar_pandora::PandoraPFParticleUtils::HasVertex(pfparticle, evt, m_recoModuleLabel)) { continue; }
            
        const std::vector<art::Ptr<recob::SpacePoint>> spacepoints = lar_pandora::PandoraPFParticleUtils::GetSpacePoints(pfparticle, evt, m_recoModuleLabel);
        if (spacepoints.empty()) { continue; }
        
        Reset();

        m_run = evt.run();
        m_subrun = evt.subRun();
        m_event = evt.event();
        m_isPrimary = (std::find(nuChildPFPs.begin(), nuChildPFPs.end(), pfparticle) != nuChildPFPs.end());

        // First, let's get the truth information...
        const std::vector<art::Ptr<recob::Hit>> pfpHits = lar_pandora::PandoraPFParticleUtils::GetHits(pfparticle, evt, m_recoModuleLabel);
        const std::vector<art::Ptr<recob::Hit>> eventHitList = lar_pandora::PandoraEventUtils::GetHits(evt, m_hitModuleLabel);
        auto const clockData = art::ServiceHandle<detinfo::DetectorClocksService>()->DataFor(evt);
        const int g4id = IvysaurusUtils::TrueParticleIDFromTotalRecoHits(clockData, pfpHits, rollUpMap);

        if (TruthMatchUtils::Valid(g4id) && (g4id >= 0))
        {
            // If it isn't a PDG that we care about, move on...
            simb::MCParticle* mcParticle = piServ->ParticleList().at(g4id);
            m_truePDG = mcParticle->PdgCode();
            const int absPDG = std::abs(m_truePDG);

            if ((absPDG != 13) && (absPDG != 2212) && (absPDG != 211) && (absPDG != 11) && (absPDG != 22) && (absPDG != 321))
                continue;
            
            // Is Michel or DR? (ignore 'primaries')
            if ((absPDG == 11) && (mcParticle->Mother() != 0))
            {
                simb::MCParticle* parentMCParticle = piServ->ParticleList().at(mcParticle->Mother());
                
                if (std::abs(parentMCParticle->PdgCode()) == 13)
                {
                    if (mcParticle->Process() == "muIoni")
                    {
                        m_isDeltaRay = true;
                    }
                    else
                    {
                        bool hasNumu = false, hasNue = false, hasElectron = false;

                        for (int iChild = 0; iChild < parentMCParticle->NumberDaughters(); ++iChild)
                        {
                            const int childTrackId = parentMCParticle->Daughter(iChild);
                            simb::MCParticle* childMCParticle = piServ->ParticleList().at(childTrackId);       
                            const int childPDG = std::abs(childMCParticle->PdgCode());

                            if (childPDG == 11)
                                hasElectron = true;
                            else if (childPDG == 12)
                                hasNue = true;
                            else if (childPDG == 14)
                                hasNumu = true;
                        }

                        m_isMichel = (hasNumu && hasNue && hasElectron);
                    }
                }
            }

            m_completeness = IvysaurusUtils::CompletenessFromTrueParticleID(clockData, rollUpMap, pfpHits, eventHitList, g4id);
            m_purity = IvysaurusUtils::HitPurityFromTrueParticleID(clockData, rollUpMap, pfpHits, g4id);
            m_trueEndX = mcParticle->EndX(); m_trueEndY = mcParticle->EndY(); m_trueEndZ = mcParticle->EndZ();
        }
        else
        {
            continue;
        }

        // Apply truth quality cuts
        if ((m_completeness < m_completenessThreshold) || (m_purity < m_purityThreshold))
            continue;

        // Fill generic vars
        if (lar_pandora::PandoraPFParticleUtils::HasTrack(pfparticle, evt, m_recoModuleLabel, m_trackModuleLabel))
        {
            const art::Ptr<recob::Track> track = lar_pandora::PandoraPFParticleUtils::GetTrack(pfparticle, evt, m_recoModuleLabel, m_trackModuleLabel);
            m_recoEndX = track->End().X(); m_recoEndY = track->End().Y(); m_recoEndZ = track->End().Z();
        }
        m_nSpacePoints = spacepoints.size();

        // Grids
        GridManager::GridMap gridMapStart = m_gridManager.ObtainGridMap(evt, pfparticle, true);
        if (gridMapStart.size() != 3) { continue; }
        GridManager::GridMap gridMapEnd = m_gridManager.ObtainGridMap(evt, pfparticle, false);
        if (gridMapEnd.size() != 3) { continue; }

        for (IvysaurusUtils::PandoraView pandoraView : {IvysaurusUtils::PandoraView::TPC_VIEW_U,
             IvysaurusUtils::PandoraView::TPC_VIEW_V, IvysaurusUtils::PandoraView::TPC_VIEW_W})
        {
            const GridManager::Grid &startGrid = gridMapStart.at(pandoraView);
            const GridManager::Grid &endGrid = gridMapEnd.at(pandoraView);
            float (&startGridValues)[576] = pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_U ? m_startGridValuesU : 
                pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_V ? m_startGridValuesV : m_startGridValuesW;
            float (&endGridValues)[576] = pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_U ? m_endGridValuesU : 
                pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_V ? m_endGridValuesV : m_endGridValuesW;

            const unsigned int dimensions = startGrid.GetAxisDimensions();
            for (unsigned int driftIndex = 0; driftIndex < dimensions; ++driftIndex)
            {
                const int index = (driftIndex * dimensions);
                
                for (unsigned int wireIndex = 0; wireIndex <  dimensions; ++wireIndex)
                {
                    startGridValues[index + wireIndex] = startGrid.GetGridValues().at(driftIndex).at(wireIndex).first;
                    endGridValues[index + wireIndex] = endGrid.GetGridValues().at(driftIndex).at(wireIndex).first;
                }
            }
        }

        // Now fill the pfp variables
        PFPVarManager::PFPVars pfpVars;
        m_pfpVarManager.EvaluatePFPVars(evt, pfparticle, pfpVars);
        m_pfpN2DHits = pfpVars.GetN2DHits();
        m_pfpTrackShowerScore = pfpVars.GetTrackShowerScore();

        // Now fill the track variables
        IvysaurusUtils::DetectorBoundaries detectorBoundaries;
        IvysaurusUtils::GetDetectorBoundaries(detectorBoundaries);
        TrackVarManager::TrackVars trackVars;
        m_trackVarsSuccessful = m_trackVarManager.EvaluateTrackVars(evt, detectorBoundaries, pfparticle, trackVars) ? 1 : 0;
        m_nTrackChildren = trackVars.GetNTrackChildren().first;
        m_nShowerChildren = trackVars.GetNShowerChildren().first;
        m_nGrandChildren = trackVars.GetNGrandChildren().first;
        m_nChildHits = trackVars.GetNChildHits().first;
        m_childEnergy = trackVars.GetChildEnergy().first;
        m_childTrackScore = trackVars.GetChildTrackScore().first;
        m_trackLength = trackVars.GetTrackLength().first;
        m_trackWobble = trackVars.GetWobble().first;
        m_trackMomComparison = trackVars.GetMomentumComparison().first;
        m_trackDistFromEdge = trackVars.GetDistanceToEdge().first;

        // Now fill the shower variables
        ShowerVarManager::ShowerVars showerVars;
        m_showerVarsSuccessful = m_showerVarManager.EvaluateShowerVars(evt, pfparticle, showerVars) ? 1 : 0;
        m_showerDisplacement = showerVars.GetDisplacement().first;
        m_DCA = showerVars.GetDCA().first;
        m_trackStubLength = showerVars.GetTrackStubLength().first;
        m_fromParentAvSep = showerVars.GetFromParentAvSep().first;
        m_fromParentChargeAsym = showerVars.GetFromParentChargeAsym().first;

        // Finally, plotting stuff
        if (m_writeVisualisationInfo)
        {
            // Grid boundaries
            for (IvysaurusUtils::PandoraView pandoraView : {IvysaurusUtils::PandoraView::TPC_VIEW_U,
                IvysaurusUtils::PandoraView::TPC_VIEW_V, IvysaurusUtils::PandoraView::TPC_VIEW_W})
            {
                const GridManager::Grid &startGrid = gridMapStart.at(pandoraView);
                const GridManager::Grid &endGrid = gridMapEnd.at(pandoraView);           
                std::vector<float> &startDriftBoundaries = pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_U ? m_startDriftBoundariesU :
                    pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_V ? m_startDriftBoundariesV : m_startDriftBoundariesW;
                std::vector<float> &endDriftBoundaries = pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_U ? m_endDriftBoundariesU :
                    pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_V ? m_endDriftBoundariesV : m_endDriftBoundariesW;
                std::vector<float> &startWireBoundaries = pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_U ? m_startWireBoundariesU : 
                    pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_V ? m_startWireBoundariesV : m_startWireBoundariesW;
                std::vector<float> &endWireBoundaries = pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_U ? m_endWireBoundariesU : 
                    pandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_V ? m_endWireBoundariesV : m_endWireBoundariesW;
            
                startDriftBoundaries = startGrid.GetDriftBoundaries();
                endDriftBoundaries = endGrid.GetDriftBoundaries();
                startWireBoundaries = startGrid.GetWireBoundaries();
                endWireBoundaries = endGrid.GetWireBoundaries();
            }
            // Hit info
            std::vector<art::Ptr<recob::Hit>> allHits = lar_pandora::PandoraPFParticleUtils::GetHits(pfparticle, evt, m_recoModuleLabel);        
            const std::vector<art::Ptr<recob::PFParticle>> pfpChildren = lar_pandora::PandoraPFParticleUtils::GetChildParticles(pfparticle, evt, m_recoModuleLabel);
            for (const art::Ptr<recob::PFParticle> &childPFP : pfpChildren)
            {
                const std::vector<art::Ptr<recob::Hit>> &childHits = lar_pandora::PandoraPFParticleUtils::GetHits(childPFP, evt, m_recoModuleLabel);
                allHits.insert(allHits.end(), childHits.begin(), childHits.end());
            }
            for (const art::Ptr<recob::Hit> hit : allHits)
            {
                const IvysaurusUtils::PandoraView thisPandoraView = IvysaurusUtils::GetPandora2DView(hit);
                std::vector<std::vector<double>> &projections = thisPandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_U ? m_projectionsU :
                    thisPandoraView == IvysaurusUtils::PandoraView::TPC_VIEW_V ? m_projectionsV : m_projectionsW;

                // Get 2D hit position
                float width = 0.f;
                TVector3 pandoraHitPosition = TVector3(0.f, 0.f, 0.f);
                IvysaurusUtils::ObtainPandoraHitPositionAndWidth(evt, hit, thisPandoraView, pandoraHitPosition, width);
                projections.push_back({pandoraHitPosition.X(), pandoraHitPosition.Y(), pandoraHitPosition.Z()});
            }
            // Spacepoints
            for (art::Ptr<recob::SpacePoint> spacepoint : spacepoints)
                m_spacePoints.push_back({spacepoint->XYZ()[0], spacepoint->XYZ()[1], spacepoint->XYZ()[2]});
        }             

        m_tree->Fill();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void IvysaurusTrainingFiles::Reset()
{
  const int defaultInt = -999;
  const float defaultFloat = -999.f;

  // General PFP
  m_run = defaultInt;
  m_subrun = defaultInt;
  m_event = defaultInt;
  m_truePDG = defaultInt;
  m_completeness = defaultFloat;
  m_purity = defaultFloat;
  m_nSpacePoints = defaultInt;
  m_trueEndX = defaultFloat;
  m_trueEndY = defaultFloat;
  m_trueEndZ = defaultFloat;
  m_recoEndX = defaultFloat;
  m_recoEndY = defaultFloat;
  m_recoEndZ = defaultFloat;
  m_isPrimary = false;
  m_isDeltaRay = false;
  m_isMichel = false;  
  // Plotting
  m_spacePoints.clear();
  m_projectionsU.clear();
  m_projectionsV.clear();
  m_projectionsW.clear();
  m_startDriftBoundariesU.clear();
  m_startDriftBoundariesV.clear();
  m_startDriftBoundariesW.clear();
  m_endDriftBoundariesU.clear();
  m_endDriftBoundariesV.clear();
  m_endDriftBoundariesW.clear();
  m_startWireBoundariesU.clear();
  m_startWireBoundariesV.clear();
  m_startWireBoundariesW.clear();
  m_endWireBoundariesU.clear();
  m_endWireBoundariesV.clear();
  m_endWireBoundariesW.clear();
  std::fill(std::begin(m_startGridValuesU), std::end(m_startGridValuesU), 0.f);
  std::fill(std::begin(m_startGridValuesV), std::end(m_startGridValuesV), 0.f);
  std::fill(std::begin(m_startGridValuesW), std::end(m_startGridValuesW), 0.f);
  std::fill(std::begin(m_endGridValuesU), std::end(m_endGridValuesU), 0.f);
  std::fill(std::begin(m_endGridValuesV), std::end(m_endGridValuesV), 0.f);
  std::fill(std::begin(m_endGridValuesW), std::end(m_endGridValuesW), 0.f);
  // PFPVars
  m_pfpN2DHits = defaultFloat;  
  m_pfpTrackShowerScore = defaultFloat;
  // TrackVars
  m_trackVarsSuccessful = 0;
  m_nTrackChildren = defaultFloat;
  m_nShowerChildren = defaultFloat;
  m_nGrandChildren = defaultFloat;
  m_nChildHits = defaultFloat;
  m_childEnergy = defaultFloat;
  m_childTrackScore = defaultFloat;
  m_trackLength = defaultFloat;
  m_trackWobble = defaultFloat;
  m_trackMomComparison = defaultFloat;
  m_trackDistFromEdge = defaultFloat;
  // ShowerVars
  m_showerVarsSuccessful = 0;
  m_showerDisplacement = defaultFloat;
  m_DCA = defaultFloat;
  m_trackStubLength = defaultFloat;
  m_fromParentAvSep = defaultFloat;
  m_fromParentChargeAsym = defaultFloat;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void IvysaurusTrainingFiles::beginJob()
{
    art::ServiceHandle<art::TFileService> tfs;
    m_tree = tfs->make<TTree>("ivysaur", "Ivysaur");
    // General PFP
    m_tree->Branch("Run", &m_run);
    m_tree->Branch("Subrun", &m_subrun);
    m_tree->Branch("Event", &m_event);
    m_tree->Branch("TruePDG", &m_truePDG);
    m_tree->Branch("Completeness", &m_completeness);
    m_tree->Branch("Purity", &m_purity);
    m_tree->Branch("NSpacePoints", &m_nSpacePoints);
    m_tree->Branch("TrueEndX", &m_trueEndX);
    m_tree->Branch("TrueEndY", &m_trueEndY);
    m_tree->Branch("TrueEndZ", &m_trueEndZ);           
    m_tree->Branch("RecoEndX", &m_recoEndX);
    m_tree->Branch("RecoEndY", &m_recoEndY);
    m_tree->Branch("RecoEndZ", &m_recoEndZ);    
    m_tree->Branch("IsPrimary", &m_isPrimary, "IsPrimary/O");
    m_tree->Branch("IsDeltaRay", &m_isDeltaRay, "IsDeltaRay/O");
    m_tree->Branch("IsMichel", &m_isMichel, "IsMichel/O");        
    // Plotting
    if (m_writeVisualisationInfo)
    {
        m_tree->Branch("SpacePoints", &m_spacePoints);
        m_tree->Branch("ProjectionsU", &m_projectionsU);
        m_tree->Branch("ProjectionsV", &m_projectionsV);
        m_tree->Branch("ProjectionsW", &m_projectionsW);
        m_tree->Branch("StartDriftBoundariesU", &m_startDriftBoundariesU);
        m_tree->Branch("StartDriftBoundariesV", &m_startDriftBoundariesV);
        m_tree->Branch("StartDriftBoundariesW", &m_startDriftBoundariesW);
        m_tree->Branch("EndDriftBoundariesU", &m_endDriftBoundariesU);
        m_tree->Branch("EndDriftBoundariesV", &m_endDriftBoundariesV);
        m_tree->Branch("EndDriftBoundariesW", &m_endDriftBoundariesW);
        m_tree->Branch("StartWireBoundariesU", &m_startWireBoundariesU);
        m_tree->Branch("StartWireBoundariesV", &m_startWireBoundariesV);
        m_tree->Branch("StartWireBoundariesW", &m_startWireBoundariesW);
        m_tree->Branch("EndWireBoundariesU", &m_endWireBoundariesU);
        m_tree->Branch("EndWireBoundariesV", &m_endWireBoundariesV);
        m_tree->Branch("EndWireBoundariesW", &m_endWireBoundariesW);
    }
    m_tree->Branch("StartGridU", m_startGridValuesU, "StartGridU[576]/F");
    m_tree->Branch("StartGridV", m_startGridValuesV, "StartGridV[576]/F");
    m_tree->Branch("StartGridW", m_startGridValuesW, "StartGridW[576]/F");
    m_tree->Branch("EndGridU", m_endGridValuesU, "EndGridU[576]/F");
    m_tree->Branch("EndGridV", m_endGridValuesV, "EndGridV[576]/F");
    m_tree->Branch("EndGridW", m_endGridValuesW, "EndGridW[576]/F");
    // PFPVars
    m_tree->Branch("PFPN2DHits", &m_pfpN2DHits);
    m_tree->Branch("PFPTrackShowerScore", &m_pfpTrackShowerScore);
    // TrackVars
    m_tree->Branch("TrackVarsSuccessful", &m_trackVarsSuccessful);
    m_tree->Branch("NTrackChildren", &m_nTrackChildren);
    m_tree->Branch("NShowerChildren", &m_nShowerChildren);
    m_tree->Branch("NGrandChildren", &m_nGrandChildren);
    m_tree->Branch("NChildHits", &m_nChildHits);
    m_tree->Branch("ChildEnergy", &m_childEnergy);
    m_tree->Branch("ChildTrackScore", &m_childTrackScore);
    m_tree->Branch("TrackLength", &m_trackLength);
    m_tree->Branch("TrackWobble", &m_trackWobble);    
    m_tree->Branch("TrackMomComparison", &m_trackMomComparison);
    m_tree->Branch("TrackDistFromEdge", &m_trackDistFromEdge);
    // ShowerVars
    m_tree->Branch("ShowerVarsSuccessful", &m_showerVarsSuccessful);
    m_tree->Branch("ShowerDisplacement", &m_showerDisplacement);
    m_tree->Branch("ShowerDCA", &m_DCA);
    m_tree->Branch("ShowerTrackStubLength", &m_trackStubLength);
    m_tree->Branch("ShowerFromParentAvSep", &m_fromParentAvSep);
    m_tree->Branch("ShowerFromParentChargeAsym", &m_fromParentChargeAsym);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void IvysaurusTrainingFiles::endJob()
{
}

} //namespace ivysaurus

