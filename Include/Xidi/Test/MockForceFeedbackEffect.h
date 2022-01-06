/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MockElementMapper.h
 *   Mock element mapper interface that can be used for tests.
 *****************************************************************************/

#pragma once

#include "ForceFeedbackEffect.h"


namespace XidiTest
{
    using namespace ::Xidi::Controller::ForceFeedback;


    /// Mock version of a force feedback effect, used for testing purposes.
    /// Simply returns the received time as the output magnitude.
    class MockEffect : public Effect
    {
    public:
        // -------- CONCRETE INSTANCE METHODS ------------------------------ //

        std::unique_ptr<Effect> Clone(void) const override
        {
            return std::make_unique<MockEffect>(*this);
        }

        TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const override
        {
            return (TEffectValue)rawTime;
        }
    };
}
