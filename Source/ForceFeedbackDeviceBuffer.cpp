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
            // -------- INTERNAL FUNCTIONS --------------------------------- //

            /// Computes the relative timestamp that corresponds to a given base and optional provided timestamp value.
            /// @param [in] timestampBase Fixed baseline timestamp.
            /// @param [in] timestamp Optional absolute timestamp value, which if absent results in the current system time being used.
            /// @return Corresponding relative timestamp.
            static inline TEffectTimeMs RelativeTimestamp(TEffectTimeMs timestampBase, std::optional<TEffectTimeMs> timestamp)
            {
                return ((true == timestamp.has_value()) ? timestamp.value() : ImportApiWinMM::timeGetTime()) - timestampBase;
            }


            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //
            // See "ForceFeedbackDeviceBuffer.h" for documentation.

            DeviceBuffer::DeviceBuffer(void) : DeviceBuffer(ImportApiWinMM::timeGetTime())
            {
                // Nothing to do here.
            }

            // --------

            DeviceBuffer::DeviceBuffer(TEffectTimeMs timestampBase) : bufferMutex(), readyEffects(), playingEffects(), timestampBase(timestampBase), timestampRelativeLastPlay()
            {
                // Nothing to do here.
            }


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

            bool DeviceBuffer::IsEffectPlaying(TEffectIdentifier id)
            {
                std::shared_lock lock(bufferMutex);

                auto playingEffectIter = playingEffects.find(id);
                if (playingEffects.end() == playingEffectIter)
                    return false;

                // This last check filters out effects that are pending playback but have not yet officially started due to a start delay.
                return (timestampRelativeLastPlay >= playingEffectIter->second.startTime);
            }

            // --------

            TOrderedMagnitudeComponents DeviceBuffer::PlayEffects(std::optional<TEffectTimeMs> timestamp)
            {
                std::unique_lock lock(bufferMutex);

                const TEffectTimeMs kRelativeTimestampPlayback = RelativeTimestamp(timestampBase, timestamp);
                timestampRelativeLastPlay = kRelativeTimestampPlayback;

                TOrderedMagnitudeComponents playbackResult = {};

                auto playingEffectIter = playingEffects.begin();
                while (playingEffectIter != playingEffects.end())
                {
                    SEffectData& effectData = playingEffectIter->second;

                    // Effects with start delays would be added to the playing effects data structure with start times in the future.
                    // This check skips playback of effects that have not officially started playing due to a start delay parameter.
                    if (kRelativeTimestampPlayback >= effectData.startTime)
                    {
                        const TEffectTimeMs kEffectPlayTime = kRelativeTimestampPlayback - effectData.startTime;

                        if (kEffectPlayTime >= effectData.effect->GetDuration())
                        {
                            // An iteration of the effect has finished playing.
                            // If there are iterations left then repeat the effect, otherwise remove it from playback.
                            if (effectData.numIterationsLeft > 0)
                            {
                                effectData.numIterationsLeft -= 1;
                                effectData.startTime = kRelativeTimestampPlayback;
                                playbackResult += effectData.effect->ComputeOrderedMagnitudeComponents(0);
                            }
                            else
                            {
                                // As soon as the playing effect is removed from its container the iterator is invalidated.
                                // Therefore, move on to the next effect before removing it.
                                // This path is the only one that bypasses the auto-increment that occurs at the end of a loop iteration.
                                auto finishedEffectIter = playingEffectIter++;
                                readyEffects.insert(playingEffects.extract(finishedEffectIter));
                                continue;
                            }
                        }
                        else
                        {
                            // Effect is currently playing.
                            // This is as simple as computing its magnitude components and adding them to the result.
                            playbackResult += effectData.effect->ComputeOrderedMagnitudeComponents(kEffectPlayTime);
                        }
                    }

                    ++playingEffectIter;
                }

                return playbackResult;
            }
            
            // --------

            bool DeviceBuffer::StartEffect(TEffectIdentifier id, unsigned int numIterations, std::optional<TEffectTimeMs> timestamp)
            {
                if (0 == numIterations)
                    return true;
                
                std::unique_lock lock(bufferMutex);

                auto readyEffectIter = readyEffects.find(id);
                if (readyEffects.end() == readyEffectIter)
                    return false;

                readyEffectIter->second.startTime = RelativeTimestamp(timestampBase, timestamp) + readyEffectIter->second.effect->GetStartDelay();
                readyEffectIter->second.numIterationsLeft = numIterations - 1;

                return playingEffects.insert(readyEffects.extract(readyEffectIter)).inserted;
            }

            // --------

            void DeviceBuffer::StopAllEffects(void)
            {
                std::unique_lock lock(bufferMutex);

                while (false == playingEffects.empty())
                    readyEffects.insert(playingEffects.extract(playingEffects.cbegin()));
            }

            // --------

            bool DeviceBuffer::StopEffect(TEffectIdentifier id)
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