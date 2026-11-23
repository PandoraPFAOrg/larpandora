include(ArtMake)
include(ArtDictionary)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

cet_make_library(
  SOURCE
    PandoraPIDResult.cxx
  LIBRARIES
    lardataobj::RecoBase
)

art_dictionary(
  SOURCE
    classes.h
    classes_def.xml
  DICTIONARY_LIBRARIES
    lardataobj::RecoBase
    larpandora_LArPandoraEventBuilding_LArPandoraPID_LArPandoraPIDData
)

install_headers()
install_source()
