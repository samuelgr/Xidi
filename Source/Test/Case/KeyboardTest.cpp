/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file VirtualControllerTest.cpp
 *   Unit tests for virtual controller objects.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "KeyboardTypes.h"
#include "TestCase.h"

//


namespace XidiTest
{
    using namespace ::Xidi;
    using ::Xidi::Controller::TControllerIdentifier;
    using ::Xidi::Controller::kPhysicalControllerCount;
    using ::Xidi::Keyboard::EKeyTransition;
    using ::Xidi::Keyboard::KeyState;
    using ::Xidi::Keyboard::TKeyIdentifier;


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies the nominal behavior in which a key is considered pressed by a varying number of controllers.
    // Whenever at least one controller says the key is pressed the state is expected to be pressed.
    TEST_CASE(KeyState_State_Nominal)
    {
        for (TControllerIdentifier numControllers = 0; numControllers <= kPhysicalControllerCount; ++numControllers)
        {
            KeyState state;
            TEST_ASSERT(false == state.IsPressed());

            for (TControllerIdentifier controller = 0; controller < numControllers; ++controller)
            {
                state.Press(controller);
                TEST_ASSERT(true == state.IsPressed());
            }

            for (TControllerIdentifier controller = 0; controller < numControllers; ++controller)
            {
                state.Release(controller);

                if (controller < (numControllers - 1))
                    TEST_ASSERT(true == state.IsPressed());
            }

            TEST_ASSERT(false == state.IsPressed());
        }
    }

    // Verifies error case behavior in which controller identifiers that are out-of-bounds are used to indicate to a key state object that the key should be pressed.
    // These requests are not expected to occur at all, but the object should ignore them if they do.
    TEST_CASE(KeyState_State_OutOfBoundsController)
    {
        constexpr TControllerIdentifier kOutOfBoundsIdentifiers[] = {kPhysicalControllerCount, kPhysicalControllerCount + 1, kPhysicalControllerCount * 2};

        KeyState state;

        for (TControllerIdentifier controllerIdentifier : kOutOfBoundsIdentifiers)
        {
            state.Press(controllerIdentifier);
            TEST_ASSERT(false == state.IsPressed());
        }
    }

    // Verifies that a key state object reports no change when there is no difference in pressed or not-pressed state between two state objects.
    // This should be true regardless of which specific controller contributed to the state of the key state object.
    TEST_CASE(KeyState_Transition_NoChange)
    {
        constexpr TControllerIdentifier kControllerIdentifiers[] = {0, 1};
        constexpr EKeyTransition kExpectedTransitionSelf = EKeyTransition::NoChange;
        constexpr EKeyTransition kExpectedTransitionForward = EKeyTransition::NoChange;
        constexpr EKeyTransition kExpectedTransitionBackward = EKeyTransition::NoChange;

        KeyState state[2];
        const EKeyTransition kActualTransitionFromReleasedForward = state[1].GetTransitionFrom(state[0]);
        const EKeyTransition kActualTransitionFromReleasedBackward = state[0].GetTransitionFrom(state[1]);
        TEST_ASSERT(kActualTransitionFromReleasedForward == kExpectedTransitionForward);
        TEST_ASSERT(kActualTransitionFromReleasedBackward == kExpectedTransitionBackward);

        state[0].Press(kControllerIdentifiers[0]);
        state[1].Press(kControllerIdentifiers[1]);
        const EKeyTransition kActualTransitionSelf = state[0].GetTransitionFrom(state[0]);
        const EKeyTransition kActualTransitionForward = state[1].GetTransitionFrom(state[0]);
        const EKeyTransition kActualTransitionBackward = state[0].GetTransitionFrom(state[1]);
        TEST_ASSERT(kActualTransitionSelf == kExpectedTransitionSelf);
        TEST_ASSERT(kActualTransitionForward == kExpectedTransitionForward);
        TEST_ASSERT(kActualTransitionBackward == kExpectedTransitionBackward);
    }

    // Verifies that a key state object reports a pressed transition when its previous state is unpressed and its current state is pressed.
    TEST_CASE(KeyState_Transition_Pressed)
    {
        constexpr TControllerIdentifier kControllerIdentifier = 2;
        constexpr EKeyTransition kExpectedTransitionForward = EKeyTransition::KeyWasPressed;
        constexpr EKeyTransition kExpectedTransitionBackward = EKeyTransition::KeyWasReleased;
        
        KeyState state[2];
        state[1].Press(kControllerIdentifier);

        const EKeyTransition kActualTransitionForward = state[1].GetTransitionFrom(state[0]);
        const EKeyTransition kActualTransitionBackward = state[0].GetTransitionFrom(state[1]);
        TEST_ASSERT(kActualTransitionForward == kExpectedTransitionForward);
        TEST_ASSERT(kActualTransitionBackward == kExpectedTransitionBackward);
    }

    // Verifies that a key state object reports a released transition when its previous state is pressed and its current state is unpressed.
    TEST_CASE(KeyState_Transition_Released)
    {
        constexpr TControllerIdentifier kControllerIdentifier = 3;
        constexpr EKeyTransition kExpectedTransitionForward = EKeyTransition::KeyWasReleased;
        constexpr EKeyTransition kExpectedTransitionBackward = EKeyTransition::KeyWasPressed;

        KeyState state[2];
        state[0].Press(kControllerIdentifier);

        const EKeyTransition kActualTransitionForward = state[1].GetTransitionFrom(state[0]);
        const EKeyTransition kActualTransitionBackward = state[0].GetTransitionFrom(state[1]);
        TEST_ASSERT(kActualTransitionForward == kExpectedTransitionForward);
        TEST_ASSERT(kActualTransitionBackward == kExpectedTransitionBackward);
    }
}
