/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ForceFeedbackDevice.h
 *   Interface declaration for objects that model the force feedback systems
 *   on physical controller devices.
 **************************************************************************************************/

#pragma once

#include <map>
#include <memory>
#include <shared_mutex>

#include "ForceFeedbackEffect.h"
#include "ForceFeedbackTypes.h"

namespace Xidi
{
  namespace Controller
  {
    namespace ForceFeedback
    {
      /// Emulates a force feedback system that would normally reside on a physical device. Includes
      /// buffers for storage and all effect playback logic. Concurrency-safe, but not safe to be
      /// constructed during dynamic initialization.
      class Device
      {
      public:

        /// Maximum number of effects that can be held in a device buffer, whether they are playing
        /// or not.
        static constexpr unsigned int kEffectMaxCount = 256;

        /// Describes an effect that is currently playing.
        struct SEffectData
        {
          /// Effect object, which defines the force magnitude at any given time.
          std::unique_ptr<Effect> effect;

          /// Relative timestamp in milliseconds at which the effect started playing.
          TEffectTimeMs startTime;

          /// Number of iterations to repeat the effect after it finishes playing.
          unsigned int numIterationsLeft;
        };

        Device(void);

        /// Allows a base timestamp to be provided, which should only ever be done during testing.
        Device(TEffectTimeMs timestampBase);

        /// Adds the specified effect into the device buffer or updates its parameters if it already
        /// exists in the device buffer. Does not check that the effect is completely defined.
        /// @param [in] effect Effect object to be added or updated.
        /// @return `true` on success, `false` on failure. This method will fail if too many effects
        /// already exist in the device buffer.
        bool AddOrUpdateEffect(const Effect& effect);

        /// Clears all effects from this device and resets any paused or muted states that might
        /// have been set.
        inline void Clear(void)
        {
          std::unique_lock lock(mutex);
          readyEffects.clear();
          playingEffects.clear();
          stateEffectsAreMuted = false;
          stateEffectsArePaused = false;
        }

        /// Retrieves and returns the number of effects that exist in the device buffer and are
        /// currently playing. For the purposes of this method call, effects are considered playing
        /// even if the device is paused and even if the effects are in their start delay period.
        /// @return Number of effects on the device that are currently playing.
        inline unsigned int GetCountPlayingEffects(void)
        {
          std::shared_lock lock(mutex);
          return (unsigned int)playingEffects.size();
        }

        /// Retrieves and returns the total number of effects that exist in the device buffer.
        /// @return Total number of effects on the device.
        inline unsigned int GetCountTotalEffects(void)
        {
          std::shared_lock lock(mutex);
          return (unsigned int)(playingEffects.size() + readyEffects.size());
        }

        /// Determines if the device is empty or not.
        /// The device is empty if no effects exist within its buffers.
        /// @return `true` if so, `false` if not.
        inline bool IsDeviceEmpty(void)
        {
          return (0 == GetCountTotalEffects());
        }

        /// Determines if the force feedback system's output state is muted.
        /// @return `true` if so, `false` otherwise.
        inline bool IsDeviceOutputMuted(void)
        {
          return stateEffectsAreMuted;
        }

        /// Determines if the force feedback system is currently paused.
        /// @return `true` if so, `false` otherwise.
        inline bool IsDeviceOutputPaused(void)
        {
          return stateEffectsArePaused;
        }

        /// Determines if the device is playing any effects or not.
        /// @return `true` if so, `false` if not.
        inline bool IsDevicePlayingAnyEffects(void)
        {
          return (0 != GetCountPlayingEffects());
        }

        /// Determines if the identified effect is loaded into the device buffer.
        /// @param [in] id Identifier of the effect of interest.
        /// @return `true` if so, `false` if not.
        inline bool IsEffectOnDevice(TEffectIdentifier id)
        {
          std::shared_lock lock(mutex);
          return (readyEffects.contains(id) || playingEffects.contains(id));
        }

        /// Determines if the identified effect is loaded into the device buffer and currently
        /// playing.
        /// @param [in] id Identifier of the effect of interest.
        /// @return `true` if so, `false` if not.
        bool IsEffectPlaying(TEffectIdentifier id);

        /// Computes the magnitude components for all of the effects that are currently playing.
        /// Any effects that are completed are automatically stopped.
        /// @param [in] timestamp Effective relative timestamp for the playback operation. Generally
        /// should not be passed (which would mean use the current time), but exposed for testing.
        /// @return Magnitude components that result from playing all of the effects at the current
        /// time.
        TOrderedMagnitudeComponents PlayEffects(
            std::optional<TEffectTimeMs> timestamp = std::nullopt);

        /// Sets the force feedback system's muted state.
        /// In muted state effects play but no output is actually produced.
        /// @param [in] muted `true` if effects should be muted, `false` otherwise.
        inline void SetMutedState(bool muted)
        {
          std::unique_lock lock(mutex);
          stateEffectsAreMuted = muted;
        }

        /// Sets the force feedback system's paused state.
        /// In paused state the effects do not play and their clocks do not advance towards their
        /// duration.
        /// @param [in] paused `true` if effects should be paused, `false` otherwise.
        inline void SetPauseState(bool paused)
        {
          std::unique_lock lock(mutex);
          stateEffectsArePaused = paused;
        }

        /// Starts playing the identified effect. If the effect is already playing, it is restarted
        /// from the beginning.
        /// @param [in] id Identifier of the effect of interest.
        /// @param [in] numIterations Number of times to repeat the effect.
        /// @param [in] timestamp Starting relative timestamp to associate with the effect.
        /// Generally should not be passed (which would mean use the current time), but exposed for
        /// testing.
        /// @return `true` on success, `false` on failure. This method will fail if the identified
        /// effect does not exist in the device buffer.
        bool StartEffect(
            TEffectIdentifier id,
            unsigned int numIterations,
            std::optional<TEffectTimeMs> timestamp = std::nullopt);

        /// Stops playing all effects that are currently playing.
        void StopAllEffects(void);

        /// Stops playing the identified effect if it is currently playing.
        /// @param [in] od Identifier of the effect of interest.
        /// @return `true` on success, `false` on failure. This method will fail if the identified
        /// effect is not currently playing.
        bool StopEffect(TEffectIdentifier id);

        /// Removes the identified effect from the device buffer. It is automatically stopped if it
        /// is currently playing.
        /// @param [in] id Identifier of the effect of interest.
        /// @return `true` on success, `false` on failure. This method will fail if the identified
        /// effect does not exist in the device buffer.
        bool RemoveEffect(TEffectIdentifier id);

      private:

        /// Enforces proper concurrency control for this object.
        std::shared_mutex mutex;

        /// Holds all force feedback effects that are available on the device but not
        /// playing.
        std::map<TEffectIdentifier, SEffectData> readyEffects;

        /// Holds all force feedback effects that are currently playing on the device.
        std::map<TEffectIdentifier, SEffectData> playingEffects;

        /// Indicates whether or not the force feedback effects are muted or not.
        /// If so, no effects produce any output but time can advance.
        bool stateEffectsAreMuted;

        /// Indicates whether playback of force feedback effects is paused or not.
        /// If so, no effects produce any output and time stops.
        bool stateEffectsArePaused;

        /// Base timestamp, used to establish a way of transforming system uptime to
        /// relative time elapsed since object creation.
        TEffectTimeMs timestampBase;

        /// Caches the relative timestamp of the last playback operation.
        TEffectTimeMs timestampRelativeLastPlay;
      };
    } // namespace ForceFeedback
  }   // namespace Controller
} // namespace Xidi
