#include "larpandora/LArPandoraEventBuilding/LArPandoraPID/LArPandoraPIDData/PandoraPIDResult.h"

namespace pandorapid
{
    
PandoraPIDResult::PandoraPIDResult()
{
}    

//------------------------------------------------------------------------------------------------------------------------------------------

std::pair<ParticleType, float> PandoraPIDResult::GetPredictedParticleType(const ParticleTypeScores &pidScores) const
{
    std::pair<ParticleType, float> prediction = std::make_pair(ParticleType::END, -1.f);
    
    for (const auto &entry : pidScores)
    {
        const ParticleType particleType = entry.first;
        const float score = entry.second;

        if (particleType == ParticleType::TRACK || particleType == ParticleType::SHOWER)
            continue;

        if ((score > prediction.second) &&
            (std::fabs(score - prediction.second) > std::numeric_limits<float>::epsilon()))              
        {
            prediction = entry;
        }
    }
    
    return prediction;
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool PandoraPIDResult::AddPIDScore(const ParticleType particleType, const float score)
{
    if (m_pidScores.find(particleType) != m_pidScores.end())
        return false;

    m_pidScores[particleType] = score;
    
    return true;
}
    
} //namespace pandorapid

