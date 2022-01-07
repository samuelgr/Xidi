/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ForceFeedbackDeviceTest.cpp
 *   Unit tests for force feedback device objects.
 *****************************************************************************/

#include "ForceFeedbackDevice.h"
#include "ForceFeedbackTypes.h"
#include "MockForceFeedbackEffect.h"
#include "TestCase.h"

#include <cstdint>
#include <limits>
#include <optional>


namespace XidiTest
{
    using namespace ::Xidi::Controller::ForceFeedback;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Default base timestamp to use for creating device buffer objects.
    static constexpr TEffectTimeMs kDefaultTimestampBase = 0;


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Initializes a device buffer object with a default base timestamp which can be overridden.
    /// A base timestamp is always supplied to make tests completely deterministic.
    /// @param [in] timestampBase Base timestamp to use instead of the default.
    /// @return Properly-initialized device buffer object.
    static Device MakeTestDevice(TEffectTimeMs timestampBase = kDefaultTimestampBase)
    {
        return Device(timestampBase);
    }
    
    /// Initializes a mock effect object using defaults for mandatory parameters.
    /// @param [in] duration Desired duration for the effect. The effect parameter will remain unset if this is not supplied.
    /// @return Properly-initialized mock effect object that can be used in test cases.
    static MockEffect MakeTestEffect(std::optional<TEffectTimeMs> duration = std::nullopt)
    {
        MockEffect effect;

        effect.InitializeDefaultAssociatedAxes();
        effect.InitializeDefaultDirection();

        if (true == duration.has_value())
            effect.SetDuration(duration.value());

        return effect;
    }


    // -------- TEST CASES ------------------------------------------------- //

    // Simple situation in which a single effect exists for playback.
    // Verifies that the correct magnitude vector is retrieved at each time.
    // At the end the effect is removed, which is expected to leave the device buffer empty.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_Nominal)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));

        // Final iteration is one past the playback duration. Effect should not be playing once the loop finishes.
        for (TEffectTimeMs t = 0; t <= kTestEffectDuration; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        TEST_ASSERT(true == Device.RemoveEffect(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectOnDevice(effect.Identifier()));
    }

    // Same simple test as above but this time the timestamps the buffer receives from the system experience an overflow.
    // This should in no way affect the output produced.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_TimestampOverflow)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;
        constexpr TEffectTimeMs kTestTimestampBase = std::numeric_limits<TEffectTimeMs>::max() - (kTestEffectDuration / 4);

        Device Device = MakeTestDevice(kTestTimestampBase);

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kTestTimestampBase));

        // Final iteration is one past the playback duration. Effect should not be playing once the loop finishes.
        for (TEffectTimeMs t = 0; t <= kTestEffectDuration; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);

            const TEffectTimeMs kPlayEffectsTime = (TEffectTimeMs)(((uint64_t)t + (uint64_t)kTestTimestampBase) & (uint64_t)std::numeric_limits<TEffectTimeMs>::max());
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(kPlayEffectsTime);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
    }

    // A single effect exists for playback but is muted halfway through.
    // It should produce no output but its clock should continue to advance.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_Mute)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));

        for (TEffectTimeMs t = 0; t < kTestEffectDuration / 2; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(false == Device.IsDeviceOutputMuted());
        Device.SetMutedState(true);
        TEST_ASSERT(true == Device.IsDeviceOutputMuted());
        
        for (TEffectTimeMs t = kTestEffectDuration / 2; t <= kTestEffectDuration; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
    }

    // A single effect exists for playback but is paused and resumed.
    // It should pick up right where it left off after being resumed.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_Pause)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;
        constexpr TEffectTimeMs kTestEffectPauseDuration = 5000;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));

        for (TEffectTimeMs t = 0; t < kTestEffectDuration / 2; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(false == Device.IsDeviceOutputPaused());
        Device.SetPauseState(true);
        TEST_ASSERT(true == Device.IsDeviceOutputPaused());

        for (TEffectTimeMs t = 0; t < kTestEffectPauseDuration; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects((kTestEffectDuration / 2) + t);
        }

        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

        TEST_ASSERT(true == Device.IsDeviceOutputPaused());
        Device.SetPauseState(false);
        TEST_ASSERT(false == Device.IsDeviceOutputPaused());

        for (TEffectTimeMs t = 0; t <= kTestEffectDuration / 2; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents((kTestEffectDuration / 2) + t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects((kTestEffectDuration / 2) + kTestEffectPauseDuration + t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
    }

    // A single effect exists for playback but has a start delay.
    // Verifies that the start delay is honored and the correct magnitude vector is retrieved at each time.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_StartDelay)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;
        constexpr TEffectTimeMs kTestEffectStartDelay = 150;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);
        TEST_ASSERT(true == effect.SetStartDelay(kTestEffectStartDelay));

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));

        // Effect is ready to go but not "playing" during the start delay period.
        for (TEffectTimeMs t = 0; t <= kTestEffectStartDelay; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        // Effect is playing now that the start delay period has passed. The "t == 0" case was covered by the final iteration of the previous loop.
        // Final iteration is one past the playback duration. Effect should not be playing once the loop finishes.
        for (TEffectTimeMs t = 1; t <= kTestEffectDuration; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t + kTestEffectStartDelay);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
    }

    // A single effect is started and then stopped some time before the duration has elapsed.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_StartAndStop)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));

        for (TEffectTimeMs t = 0; t < (kTestEffectDuration / 4); ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(true == Device.StopEffect(effect.Identifier()));
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
        const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(kTestEffectDuration / 4);
        TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
    }

    // A single effect is started and then stopped some time before the duration has elapsed.
    // This time, the stop request is based on stopping all playing effects, not a specific one.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_StartAndStopAll)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));

        for (TEffectTimeMs t = 0; t < (kTestEffectDuration / 4); ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        Device.StopAllEffects();
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
        const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(kTestEffectDuration / 4);
        TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
    }

    // A single effect is started and then stopped some time before the duration has elapsed.
    // This time, the stop request is based on clearing out all effects in the buffer.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_StartAndClear)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));

        for (TEffectTimeMs t = 0; t < (kTestEffectDuration / 4); ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        Device.Clear();
        TEST_ASSERT(false == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

        const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
        const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(kTestEffectDuration / 4);
        TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
    }

    // A single effect is started and then its duration is shortened sometime before the effect stops on its own.
    // This should cause the effect to stop playing.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_StartAndShorten)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));

        for (TEffectTimeMs t = 0; t < (kTestEffectDuration / 4); ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration / 4));
        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));

        // At this point the effect is still playing.
        // Next time a magnitude is requested, at half-duration, it will be stopped and the magnitude should be 0.
        const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
        const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(kTestEffectDuration / 4);
        TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
    }

    // A single effect is started with multiple iterations.
    // Verifies that the correct magnitude vector is retrieved at each time.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_MultipleIterations)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;
        constexpr unsigned int kTestNumIterations = 5;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), kTestNumIterations, kDefaultTimestampBase));

        for (unsigned int i = 0; i < kTestNumIterations; ++i)
        {
            TEffectTimeMs kTimeBase = (i * kTestEffectDuration);

            for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
            {
                TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
                TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

                const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
                const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t + kTimeBase);
                TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
            }
        }

        // At this point the effect is still playing.
        // However, it should stop next time a magnitude is requested.
        const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
        const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(kTestEffectDuration * kTestNumIterations);
        TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
    }

    // A single effect is started with multiple iterations and a start delay.
    // Verifies that the correct magnitude vector is retrieved at each time.
    TEST_CASE(ForceFeedbackDevice_SingleEffect_MultipleIterationsStartDelay)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;
        constexpr TEffectTimeMs kTestEffectStartDelay = 150;
        constexpr unsigned int kTestNumIterations = 5;

        Device Device = MakeTestDevice();

        MockEffect effect = MakeTestEffect(kTestEffectDuration);
        TEST_ASSERT(true == effect.SetStartDelay(kTestEffectStartDelay));

        TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
        TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), kTestNumIterations, kDefaultTimestampBase));

        // Effect is ready to go but not "playing" during the start delay period.
        for (TEffectTimeMs t = 0; t <= kTestEffectStartDelay; ++t)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

            const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        }

        // Now the effect should play for the requested number of iterations.
        for (unsigned int i = 0; i < kTestNumIterations; ++i)
        {
            TEffectTimeMs kTimeBase = (i * kTestEffectDuration) + kTestEffectStartDelay;

            for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
            {
                TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
                TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));

                const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = effect.ComputeOrderedMagnitudeComponents(t);
                const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t + kTimeBase);
                TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
            }
        }

        // At this point the effect is still playing.
        // However, it should stop next time a magnitude is requested.
        const TOrderedMagnitudeComponents kExpectedMagnitudeComponents = {};
        const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects((kTestEffectDuration * kTestNumIterations) + kTestEffectStartDelay);
        TEST_ASSERT(kActualMagnitudeComponents == kExpectedMagnitudeComponents);
        TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
        TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
    }

    // Simple situation in which multiple effects exist for playback.
    // Durations are all the same, so the only real difference is that the buffer must combine the magnitudes.
    TEST_CASE(ForceFeedbackDevice_MultipleEffects_Nominal)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;

        Device Device = MakeTestDevice();

        MockEffect effects[] = {MakeTestEffect(kTestEffectDuration), MakeTestEffect(kTestEffectDuration), MakeTestEffect(kTestEffectDuration)};

        for (const MockEffect& effect : effects)
        {
            TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
            TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));
        }

        for (TEffectTimeMs t = 0; t <= kTestEffectDuration; ++t)
        {
            TOrderedMagnitudeComponents expectedMagnitudeComponents = {};

            for (const MockEffect& effect : effects)
            {
                TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
                TEST_ASSERT(true == Device.IsEffectPlaying(effect.Identifier()));
                expectedMagnitudeComponents += effect.ComputeOrderedMagnitudeComponents(t);
            }

            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == expectedMagnitudeComponents);
        }

        for (const MockEffect& effect : effects)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));

            TEST_ASSERT(true == Device.RemoveEffect(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectOnDevice(effect.Identifier()));
        }
    }

    // Simple situation in which multiple effects exist for playback.
    // Durations are all different this time.
    TEST_CASE(ForceFeedbackDevice_MultipleEffects_DifferentDurations)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;

        Device Device = MakeTestDevice();

        MockEffect effects[] = {MakeTestEffect(kTestEffectDuration), MakeTestEffect(kTestEffectDuration / 2), MakeTestEffect(kTestEffectDuration / 3)};

        for (const MockEffect& effect : effects)
        {
            TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
            TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));
        }

        for (TEffectTimeMs t = 0; t <= kTestEffectDuration; ++t)
        {
            TOrderedMagnitudeComponents expectedMagnitudeComponents = {};

            for (const MockEffect& effect : effects)
                expectedMagnitudeComponents += effect.ComputeOrderedMagnitudeComponents(t);

            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == expectedMagnitudeComponents);
        }

        for (const MockEffect& effect : effects)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
        }
    }

    // Simple situation in which multiple effects exist for playback.
    // Durations and start delays are all different this time.
    TEST_CASE(ForceFeedbackDevice_MultipleEffects_DifferentDurationsAndStartDelays)
    {
        constexpr TEffectTimeMs kTestEffectDuration = 100;
        constexpr TEffectTimeMs kTestEffectStartDelay = 150;

        Device Device = MakeTestDevice();

        MockEffect effects[] = {MakeTestEffect(kTestEffectDuration), MakeTestEffect(kTestEffectDuration / 2), MakeTestEffect(kTestEffectDuration / 3)};

        for (unsigned int i = 0; i < _countof(effects); ++i)
            effects[i].SetStartDelay(kTestEffectStartDelay / (_countof(effects) - i));
        
        for (MockEffect& effect : effects)
        {
            TEST_ASSERT(true == Device.AddOrUpdateEffect(effect));
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
            TEST_ASSERT(true == Device.StartEffect(effect.Identifier(), 1, kDefaultTimestampBase));
        }

        for (TEffectTimeMs t = 0; t <= (kTestEffectDuration + kTestEffectStartDelay); ++t)
        {
            TOrderedMagnitudeComponents expectedMagnitudeComponents = {};

            for (const MockEffect& effect : effects)
            {
                if (t >= effect.GetStartDelay())
                    expectedMagnitudeComponents += effect.ComputeOrderedMagnitudeComponents(t - effect.GetStartDelay());
            }

            const TOrderedMagnitudeComponents kActualMagnitudeComponents = Device.PlayEffects(t);
            TEST_ASSERT(kActualMagnitudeComponents == expectedMagnitudeComponents);
        }

        for (const MockEffect& effect : effects)
        {
            TEST_ASSERT(true == Device.IsEffectOnDevice(effect.Identifier()));
            TEST_ASSERT(false == Device.IsEffectPlaying(effect.Identifier()));
        }
    }
}
