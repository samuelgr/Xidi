/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ForceFeedbackBuffer.cpp
 *   Implementation of functionality related to emulating force feedback systems on physical
 *   controller devices.
 **************************************************************************************************/

#include "ForceFeedbackDevice.h"

#include <memory>
#include <mutex>

#include "ForceFeedbackEffect.h"
#include "ForceFeedbackTypes.h"
#include "ImportApiWinMM.h"

namespace Xidi
{
  namespace Controller
  {
    namespace ForceFeedback
    {
      /// Computes the relative timestamp that corresponds to a given base and optional provided
      /// timestamp value.
      /// @param [in] timestampBase Fixed baseline timestamp.
      /// @param [in] timestamp Optional absolute timestamp value, which if absent results in the
      /// current system time being used.
      /// @return Corresponding relative timestamp.
      static inline TEffectTimeMs RelativeTimestamp(
          TEffectTimeMs timestampBase, std::optional<TEffectTimeMs> timestamp)
      {
        return ((true == timestamp.has_value()) ? timestamp.value()
                                                : ImportApiWinMM::timeGetTime()) -
            timestampBase;
      }

      Device::Device(void) : Device(ImportApiWinMM::timeGetTime()) {}

      Device::Device(TEffectTimeMs timestampBase)
          : mutex(),
            readyEffects(),
            playingEffects(),
            stateEffectsAreMuted(),
            stateEffectsArePaused(),
            timestampBase(timestampBase),
            timestampRelativeLastPlay()
      {}

      bool Device::AddOrUpdateEffect(const Effect& effect)
      {
        std::unique_lock lock(mutex);

        auto playingEffectIter = playingEffects.find(effect.Identifier());
        if (playingEffects.end() != playingEffectIter)
          return playingEffectIter->second.effect->SyncParametersFrom(effect);

        auto readyEffectIter = readyEffects.find(effect.Identifier());
        if (readyEffects.end() != readyEffectIter)
          return readyEffectIter->second.effect->SyncParametersFrom(effect);

        if ((playingEffects.size() + readyEffects.size()) >= kEffectMaxCount) return false;

        readyEffects.emplace(std::make_pair(effect.Identifier(), effect.Clone()));

        return true;
      }

      bool Device::IsEffectPlaying(TEffectIdentifier id)
      {
        std::shared_lock lock(mutex);

        auto playingEffectIter = playingEffects.find(id);
        if (playingEffects.end() == playingEffectIter) return false;

        // This last check filters out effects that are pending playback but have not yet officially
        // started due to a start delay.
        return (timestampRelativeLastPlay >= playingEffectIter->second.startTime);
      }

      TOrderedMagnitudeComponents Device::PlayEffects(std::optional<TEffectTimeMs> timestamp)
      {
        std::unique_lock lock(mutex);

        const TEffectTimeMs relativeTimestampPlayback = RelativeTimestamp(timestampBase, timestamp);

        if (true == stateEffectsArePaused)
        {
          timestampBase += (relativeTimestampPlayback - timestampRelativeLastPlay);
          return {};
        }

        timestampRelativeLastPlay = relativeTimestampPlayback;

        TOrderedMagnitudeComponents playbackResult = {};

        auto playingEffectIter = playingEffects.begin();
        while (playingEffectIter != playingEffects.end())
        {
          SEffectData& effectData = playingEffectIter->second;

          // Effects with start delays would be added to the playing effects data structure with
          // start times in the future. This check skips playback of effects that have not
          // officially started playing due to a start delay parameter.
          if (relativeTimestampPlayback >= effectData.startTime)
          {
            const TEffectTimeMs effectPlayTime = relativeTimestampPlayback - effectData.startTime;

            if (effectPlayTime >= effectData.effect->GetDuration())
            {
              // An iteration of the effect has finished playing.
              // If there are iterations left then repeat the effect, otherwise remove it from
              // playback.
              if (effectData.numIterationsLeft > 0)
              {
                effectData.numIterationsLeft -= 1;
                effectData.startTime = relativeTimestampPlayback;

                if (false == stateEffectsAreMuted)
                  playbackResult += effectData.effect->ComputeOrderedMagnitudeComponents(0);
              }
              else
              {
                // As soon as the playing effect is removed from its container the iterator is
                // invalidated. Therefore, move on to the next effect before removing it. This path
                // is the only one that bypasses the auto-increment that occurs at the end of a loop
                // iteration.
                auto finishedEffectIter = playingEffectIter++;
                readyEffects.insert(playingEffects.extract(finishedEffectIter));
                continue;
              }
            }
            else
            {
              // Effect is currently playing.
              // This is as simple as computing its magnitude components and adding them to the
              // result.
              if (false == stateEffectsAreMuted)
                playbackResult +=
                    effectData.effect->ComputeOrderedMagnitudeComponents(effectPlayTime);
            }
          }

          ++playingEffectIter;
        }

        return playbackResult;
      }

      bool Device::StartEffect(
          TEffectIdentifier id, unsigned int numIterations, std::optional<TEffectTimeMs> timestamp)
      {
        if (0 == numIterations) return true;

        std::unique_lock lock(mutex);

        auto readyEffectIter = readyEffects.find(id);
        if (readyEffects.end() == readyEffectIter) return false;

        readyEffectIter->second.startTime = RelativeTimestamp(timestampBase, timestamp) +
            readyEffectIter->second.effect->GetStartDelay();
        readyEffectIter->second.numIterationsLeft = numIterations - 1;

        return playingEffects.insert(readyEffects.extract(readyEffectIter)).inserted;
      }

      void Device::StopAllEffects(void)
      {
        std::unique_lock lock(mutex);

        while (false == playingEffects.empty())
          readyEffects.insert(playingEffects.extract(playingEffects.cbegin()));
      }

      bool Device::StopEffect(TEffectIdentifier id)
      {
        std::unique_lock lock(mutex);

        auto playingEffectIter = playingEffects.find(id);
        if (playingEffects.end() == playingEffectIter) return false;

        return readyEffects.insert(playingEffects.extract(playingEffectIter)).inserted;
      }

      bool Device::RemoveEffect(TEffectIdentifier id)
      {
        std::unique_lock lock(mutex);

        if (0 != readyEffects.erase(id)) return true;

        if (0 != playingEffects.erase(id)) return true;

        return false;
      }
    } // namespace ForceFeedback
  }   // namespace Controller
} // namespace Xidi
