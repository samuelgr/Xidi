/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ForceFeedbackDeviceBuffer.cpp
 *   Implementation of functionality related to emulating force feedback
 *   effect buffers on physical controller devices.
 *****************************************************************************/

#include "ForceFeedbackDeviceBuffer.h"
#include "ForceFeedbackEffect.h"
#include "ForceFeedbackTypes.h"
#include "ImportApiWinMM.h"

#include <memory>
#include <mutex>


namespace Xidi
{
    namespace Controller
    {
        namespace ForceFeedback
        {
            // -------- INSTANCE METHODS ----------------------------------- //
            // See "ForceFeedbackDeviceBuffer.h" for documentation.

            bool DeviceBuffer::AddOrUpdateEffect(const Effect& effect)
            {
                std::unique_lock lock(bufferMutex);

                auto playingEffectIter = playingEffects.find(effect.Identifier());
                if (playingEffects.end() != playingEffectIter)
                    return playingEffectIter->second.effect->SyncParametersFrom(effect);

                auto readyEffectIter = readyEffects.find(effect.Identifier());
                if (readyEffects.end() != readyEffectIter)
                    return readyEffectIter->second.effect->SyncParametersFrom(effect);

                if ((playingEffects.size() + readyEffects.size()) >= kEffectMaxCount)
                    return false;

                readyEffects.emplace(std::make_pair(effect.Identifier(), effect.Clone()));

                return true;
            }

            // --------

            TOrderedMagnitudeComponents DeviceBuffer::GetPlayingEffectsMagnitude(void)
            {
                std::unique_lock lock(bufferMutex);

                // TODO

                return {};
            }
            
            // --------

            bool DeviceBuffer::StartPlayingEffect(TEffectIdentifier id, unsigned int numIterations)
            {
                if (0 == numIterations)
                    return true;
                
                std::unique_lock lock(bufferMutex);

                auto readyEffectIter = readyEffects.find(id);
                if (readyEffects.end() == readyEffectIter)
                    return false;

                readyEffectIter->second.startTime = ImportApiWinMM::timeGetTime();
                readyEffectIter->second.numIterationsLeft = numIterations - 1;

                return playingEffects.insert(readyEffects.extract(readyEffectIter)).inserted;
            }

            // --------

            void DeviceBuffer::StopPlayingAllEffects(void)
            {
                std::unique_lock lock(bufferMutex);

                while (false == playingEffects.empty())
                    readyEffects.insert(playingEffects.extract(playingEffects.cbegin()));
            }

            // --------

            bool DeviceBuffer::StopPlayingEffect(TEffectIdentifier id)
            {
                std::unique_lock lock(bufferMutex);

                auto playingEffectIter = playingEffects.find(id);
                if (playingEffects.end() == playingEffectIter)
                    return false;

                return readyEffects.insert(playingEffects.extract(playingEffectIter)).inserted;
            }

            // --------

            bool DeviceBuffer::RemoveEffect(TEffectIdentifier id)
            {
                std::unique_lock lock(bufferMutex);

                if (0 != readyEffects.erase(id))
                    return true;

                if (0 != playingEffects.erase(id))
                    return true;

                return false;
            }
        }
    }
}
