/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ForceFeedbackDeviceBuffer.h
 *   Interface declaration for objects that model force feedback effect
 *   buffers on physical controller devices.
 *****************************************************************************/

#pragma once

#include "ForceFeedbackEffect.h"
#include "ForceFeedbackTypes.h"

#include <map>
#include <memory>
#include <shared_mutex>


namespace Xidi
{
    namespace Controller
    {
        namespace ForceFeedback
        {
            /// Emulates a hardware buffer that would normally hold force feedback effects on a physical device.
            /// Concurrency-safe.
            class DeviceBuffer
            {
            public:
                // -------- CONSTANTS -------------------------------------- //

                /// Maximum number of effects that can be held in a device buffer, whether they are playing or not.
                static constexpr unsigned int kEffectMaxCount = 256;


            private:
                // -------- TYPE DEFINITIONS ------------------------------- //

                /// Describes an effect that is currently playing.
                struct SEffectData
                {
                    std::unique_ptr<Effect> effect;                         ///< Effect object, which defines the force magnitude at any given time.
                    TEffectTimeMs startTime;                                ///< Relative timestamp in milliseconds at which the effect started playing.
                    unsigned int numIterationsLeft;                         ///< Number of iterations to repeat the effect after it finishes playing.
                };


                // -------- INSTANCE VARIABLES ----------------------------- //

                /// Enforces proper concurrency control for this object.
                std::shared_mutex bufferMutex;

                /// Holds all force feedback effects that are available on the device but not playing.
                std::map<TEffectIdentifier, SEffectData> readyEffects;

                /// Holds all force feedback effects that are currently playing on the device.
                std::map<TEffectIdentifier, SEffectData> playingEffects;

                /// Base timestamp, set at object creation and never changes.
                /// Used to establish a way of transforming system uptime to relative time elapsed since object creation.
                const TEffectTimeMs timestampBase;

                /// Caches the relative timestamp of the last playback operation.
                TEffectTimeMs timestampRelativeLastPlay;


            public:
                // -------- CONSTRUCTION AND DESTRUCTION ------------------- //

                /// Default constructor.
                DeviceBuffer(void);

                /// Initialization constructor.
                /// Allows a base timestamp to be provided, which should only ever be done during testing.
                DeviceBuffer(TEffectTimeMs timestampBase);


                // -------- INSTANCE METHODS ------------------------------- //

                /// Adds the specified effect into the device buffer or updates its parameters if it already exists in the device buffer.
                /// Does not check that the effect is completely defined.
                /// @param [in] effect Effect object to be added or updated.
                /// @return `true` on success, `false` on failure. This method will fail if too many effects already exist in the device buffer.
                bool AddOrUpdateEffect(const Effect& effect);

                /// Clears all effects from this buffer.
                inline void Clear(void)
                {
                    std::unique_lock lock(bufferMutex);
                    readyEffects.clear();
                    playingEffects.clear();
                }

                /// Determines if the identified effect is loaded into the device buffer.
                /// @param [in] id Identifier of the effect of interest.
                /// @return `true` if so, `false` if not.
                inline bool IsEffectOnDevice(TEffectIdentifier id)
                {
                    std::shared_lock lock(bufferMutex);
                    return (readyEffects.contains(id) || playingEffects.contains(id));
                }

                /// Determines if the identified effect is loaded into the device buffer and currently playing.
                /// @param [in] id Identifier of the effect of interest.
                /// @return `true` if so, `false` if not.
                bool IsEffectPlaying(TEffectIdentifier id);

                /// Computes the magnitude components for all of the effects that are currently playing.
                /// Any effects that are completed are automatically stopped.
                /// @param [in] timestamp Effective relative timestamp for the playback operation. Generally should not be passed (which would mean use the current time), but exposed for testing.
                /// @return Magnitude components that result from playing all of the effects at the current time.
                TOrderedMagnitudeComponents PlayEffects(std::optional<TEffectTimeMs> timestamp = std::nullopt);

                /// Starts playing the identified effect. If the effect is already playing, it is restarted from the beginning.
                /// @param [in] id Identifier of the effect of interest.
                /// @param [in] numIterations Number of times to repeat the effect.
                /// @param [in] timestamp Starting relative timestamp to associate with the effect. Generally should not be passed (which would mean use the current time), but exposed for testing.
                /// @return `true` on success, `false` on failure. This method will fail if the identified effect does not exist in the device buffer.
                bool StartEffect(TEffectIdentifier id, unsigned int numIterations, std::optional<TEffectTimeMs> timestamp = std::nullopt);

                /// Stops playing all effects that are currently playing.
                void StopAllEffects(void);

                /// Stops playing the identified effect if it is currently playing.
                /// @param [in] od Identifier of the effect of interest.
                /// @return `true` on success, `false` on failure. This method will fail if the identified effect is not currently playing.
                bool StopEffect(TEffectIdentifier id);

                /// Removes the identified effect from the device buffer. It is automatically stopped if it is currently playing.
                /// @param [in] id Identifier of the effect of interest.
                /// @return `true` on success, `false` on failure. This method will fail if the identified effect does not exist in the device buffer.
                bool RemoveEffect(TEffectIdentifier id);
            };
        }
    }
}
