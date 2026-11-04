cet_make_library(LIBRARY_NAME LArPandora INTERFACE
  SOURCE ILArPandora.h
  LIBRARIES INTERFACE
  art_plugin_types::EDProducer
  canvas::canvas
)

cet_make_library(LIBRARY_NAME GeometryComponents INTERFACE
  SOURCE LArPandoraGeometryComponents.h
  LIBRARIES INTERFACE larcoreobj::SimpleTypesAndConstants
)

add_subdirectory(Detectors)

cet_make_library(LIBRARY_NAME_VAR lib_target
  SOURCE
  LArPandora.cxx
  LArPandoraGeometry.cxx
  LArPandoraHelper.cxx
  LArPandoraInput.cxx
  LArPandoraOutput.cxx
  LIBRARIES
  PUBLIC
  larpandora::LArPandora
  larpandora::GeometryComponents
  larpandoracontent::LArPandoraContent
  lardata::AssociationUtil
  lardata::Utilities
  lardataobj::AnalysisBase
  lardataobj::RecoBase
  lardataobj::Simulation
  larcoreobj::SimpleTypesAndConstants
  nusimdata::SimulationBase
  art::Persistency_Common
  canvas::canvas
  PandoraPFA::PandoraSDK
  PRIVATE
  larpandora::GetDetectorType
  larreco::ClusterFinder
  larreco::ClusterParamsImportWrapper
  larreco::RecoAlg_ClusterRecoUtil
  larevt::ChannelStatusProvider
  larevt::ChannelStatusService
  larsim::MCCheater_ParticleInventoryService_service
  larsim::MCCheater_PhotonBackTrackerService_service
  lardata::DetectorClocksService
  lardata::DetectorPropertiesService
  larcore::Geometry_Geometry_service
  larcorealg::Geometry
  art_root_io::TFileService_service
  art::Framework_Principal
  art::Framework_Services_Registry
  messagefacility::MF_MessageLogger
  cetlib::cetlib
  cetlib_except::cetlib_except
)

cet_write_plugin_builder(lar::LArPandora art::EDProducer Modules
  INSTALL_BUILDER
  LIBRARIES CONDITIONAL ${lib_target}
)

include(lar::LArPandora)

cet_build_plugin(StandardPandora lar::LArPandora
  IMPL_TARGET_VAR module_target
  LIBRARIES PRIVATE
  ${lib_target}
  larpandoracontent::LArPandoraContent
  cetlib_except::cetlib_except
  PandoraPFA::PandoraSDK
)

cet_build_plugin(LArPandoraHitCollectionToolDefault art::tool
  LIBRARIES PRIVATE
  ${lib_target}
  PandoraPFA::PandoraSDK
)

if (LARPANDORA_LIBTORCH)
  target_link_libraries(${module_target} PRIVATE larpandoracontent::LArPandoraDLContent)
  target_compile_definitions(${module_target} PRIVATE LIBTORCH_DL)
endif()

add_subdirectory(scripts)

install_headers()
install_fhicl()
install_source()
