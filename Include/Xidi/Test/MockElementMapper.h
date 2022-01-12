/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MockElementMapper.h
 *   Mock element mapper interface that can be used for tests.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "TestCase.h"

#include <memory>
#include <vector>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    /// Mock version of an element mapper, used for testing purposes to ensure that values read from a controller are correctly routed.
    class MockElementMapper : public IElementMapper
    {
    public:
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Enumerates possible expected sources of input values from an XInput controller.
        /// Specifies which of the `Contribute` methods is expected to be invoked.
        enum class EExpectedSource
        {
            None,
            Analog,
            Button,
            Trigger,
            Neutral
        };

        /// Holds an expected input value, one for each allowed type.
        union UExpectedValue
        {
            int16_t analog;
            bool button;
            uint8_t trigger;

            constexpr inline UExpectedValue(int16_t analog) : analog(analog) {}
            constexpr inline UExpectedValue(bool button) : button(button) {}
            constexpr inline UExpectedValue(uint8_t trigger) : trigger(trigger) {}
        };


    private:
        // -------- INSTANCE VARIABLES --------------------------------- //

        /// Specifies the expected source of an input value.
        /// Causes a test to fail if the wrong `ContributeFrom` method is invoked on this object.
        const EExpectedSource expectedSource;

        /// Specifies the expected input value, one for each allowed type.
        /// Which member is valid is determined by the value of #expectedSource.
        const UExpectedValue expectedValue;

        /// Holds the address of a counter that is incremented by 1 whenever this element mapper is asked for a contribution.
        int* const contributionCounter;

        /// Holds the fake list of target elements.
        const std::vector<SElementIdentifier> fakeTargetElements;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //
        
        /// Initialization constructor.
        /// Requires that values for all instance variables be provided.
        /// For tests that do not exercise controller capabilities, type and index can be omitted.
        inline MockElementMapper(EExpectedSource expectedSource, UExpectedValue expectedValue, int* contributionCounter = nullptr, std::vector<SElementIdentifier>&& fakeTargetElements = {SElementIdentifier()}) : expectedSource(expectedSource), expectedValue(expectedValue), contributionCounter(contributionCounter), fakeTargetElements(std::move(fakeTargetElements))
        {
            // Nothing to do here.
        }

        /// Initialization constructor.
        /// For simpler tests that expect no contributions but require only a single target element.
        /// Can be used as a default constructor to create a mock element identifier with empty target element.
        inline MockElementMapper(SElementIdentifier fakeTargetElement = SElementIdentifier()) : MockElementMapper(EExpectedSource::None, false, nullptr, {fakeTargetElement})
        {
            // Nothing to do here.
        }


        // -------- CONCRETE INSTANCE METHODS -------------------------- //

        std::unique_ptr<IElementMapper> Clone(void) const override
        {
            return std::make_unique<MockElementMapper>(*this);
        }

        // --------

        void ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const override
        {
            if (EExpectedSource::Analog != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected enumerator %d, got Analog).", (int)expectedSource);

            if (expectedValue.analog != analogValue)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong analog value (expected %d, got %d).", (int)expectedValue.analog, (int)analogValue);

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        // --------

        void ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const override
        {
            if (EExpectedSource::Button != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected enumerator %d, got Button).", (int)expectedSource);

            if (expectedValue.button != buttonPressed)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong button value (expected %s, got %s).", (true == expectedValue.button ? L"'true (pressed)'" : L"'false (not pressed)'"), (true == buttonPressed ? L"'true (pressed)'" : L"'false (not pressed)'"));

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        // --------

        void ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const override
        {
            if (EExpectedSource::Trigger != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected enumerator %d, got Trigger).", (int)expectedSource);

            if (expectedValue.trigger != triggerValue)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong trigger value (expected %d, got %d).", (int)expectedValue.trigger, (int)triggerValue);

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        // --------

        void ContributeNeutral(SState& controllerState) const override
        {
            // Neutral contributions are non-destructive.
            // Some element mappers will forward these to sub-element mappers, so unless explicitly testing for neutral contributions they can largely be ignored.
            // The exception is if no contributions whatsoever are expected, in which case any contributions are errors.

            if (EExpectedSource::None == expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected enumerator %d, got Trigger).", (int)expectedSource);

            if (EExpectedSource::Neutral == expectedSource)
            {
                if (nullptr != contributionCounter)
                    *contributionCounter += 1;
            }
        }

        // --------

        int GetTargetElementCount(void) const override
        {
            return (int)fakeTargetElements.size();
        }

        // --------

        std::optional<SElementIdentifier> GetTargetElementAt(int index) const override
        {
            if ((size_t)index < fakeTargetElements.size())
                return fakeTargetElements[index];

            return std::nullopt;
        }
    };
}
