/**
 *  @file  larpandora/LArPandoraEventBuilding/LArPandoraPID/LArPandoraPIDData/PandoraPIDResult.h
 *
 *  @brief A class to hold the scores of various Pandora PID estimators
 */
#ifndef PANDORA_PID_RESULT_H
#define PANDORA_PID_RESULT_H

#include <map>
#include <cmath>
#include <limits>

namespace pandorapid
{
enum ParticleType {
    MUON,
    PROTON,
    PION,
    KAON,
    ELECTRON,
    MICHELDR,
    PHOTON,
    DIFFUSE,
    MIP,
    HIP,
    EM_TRACK,
    EM_SHOWER,
    TRACK,
    SHOWER,
    END
};

typedef std::map<ParticleType, float> ParticleTypeScores;
    
/**
 *  @brief  PandoraPIDResult class
 */        
class PandoraPIDResult
{
public:
    /**
     *  @brief  Default constructor 
     */    
    PandoraPIDResult();

    /**
     *  @brief  Given an input ParticleType->score map, return the ParticleType with the highest score
     *
     *  @param  pidScores the input ParticleType->score map
     *
     *  @return the predicted ParticleType alongside its score
     */    
    std::pair<ParticleType, float> GetPredictedParticleType(const ParticleTypeScores &pidScores) const;

    /**
     *  @brief Get the ParticleType -> score map
     *
     *  @return the ParticleType -> score map
     */    
    const ParticleTypeScores& GetPIDScores() const;

    /**
     *  @brief Add a PID score for a given ParticleType to the ParticleType -> score map
     *
     *  @param particleType the particle type from the ParticleType enum
     *  @param score the score to add
     *
     *  @return whether the score could be added
     */    
    bool AddPIDScore(const ParticleType particleType, const float score);

private:    
    ParticleTypeScores m_pidScores; ///< The ParticleType->score map for the pandora-based PIDs
};

//------------------------------------------------------------------------------------------------------------------------------------------
    
inline const ParticleTypeScores& PandoraPIDResult::GetPIDScores() const
{
    return m_pidScores;
}

} // namespace pandorapid

#endif

