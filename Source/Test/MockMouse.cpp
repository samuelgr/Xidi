/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MockMouse.cpp
 *   Implementation of a mock version of the mouse interface along with
 *   additional testing-specific functions.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "MockMouse.h"
#include "TestCase.h"

#include <mutex>
#include <unordered_map>


namespace XidiTest
{
    using namespace ::Xidi::Mouse;


    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Holds the mock mouse object that is capturing input from the mouse interface functions.
    static MockMouse* capturingVirtualMouse = nullptr;

    /// For ensuring proper concurrency control over the virtual mouse capture state.
    static std::mutex captureGuard;


    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "MockMouse.h" for documentation.

    MockMouse::~MockMouse(void)
    {
        if (this == capturingVirtualMouse)
            EndCapture();
    }


    // -------- INSTANCE METHODS ------------------------------------------- //

    void MockMouse::BeginCapture(void)
    {
        std::scoped_lock lock(captureGuard);

        if (nullptr != capturingVirtualMouse)
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to attempting to replace another mock mouse already capturing events.", __FUNCTIONW__);

        capturingVirtualMouse = this;
    }

    // --------

    void MockMouse::EndCapture(void)
    {
        std::scoped_lock lock(captureGuard);

        if (this != capturingVirtualMouse)
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to attempting to end capture for a mock mouse not currently capturing events.", __FUNCTIONW__);

        capturingVirtualMouse = nullptr;
    }

    // --------

    std::optional<int> MockMouse::GetMovementContributionFromSource(EMouseAxis axis, uint32_t sourceIdentifier) const
    {
        const auto contributionIter = virtualMouseMovementContributionBySource[(unsigned int)axis].find(sourceIdentifier);
        if (virtualMouseMovementContributionBySource[(unsigned int)axis].cend() == contributionIter)
            return std::nullopt;

        return contributionIter->second;
    }

    // --------

    void MockMouse::SubmitMouseButtonPressedState(EMouseButton button)
    {
        if ((unsigned int)button >= virtualMouseButtonState.max_size())
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to out-of-bounds mouse button identifier.", __FUNCTIONW__);

        virtualMouseButtonState.insert((unsigned int)button);
    }

    // --------

    void MockMouse::SubmitMouseButtonReleasedState(EMouseButton button)
    {
        if ((unsigned int)button >= virtualMouseButtonState.max_size())
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to out-of-bounds mouse button identifier.", __FUNCTIONW__);

        virtualMouseButtonState.erase((unsigned int)button);
    }

    // --------

    void MockMouse::SubmitMouseMovement(EMouseAxis axis, int mouseMovementUnits, uint32_t sourceIdentifier)
    {
        if (mouseMovementUnits < kMouseMovementUnitsMin)
            TEST_FAILED_BECAUSE(L"%s: Mouse movement units %d is below the minimum allowed %d.", __FUNCTIONW__, mouseMovementUnits, kMouseMovementUnitsMin);

        if (mouseMovementUnits > kMouseMovementUnitsMax)
            TEST_FAILED_BECAUSE(L"%s: Mouse movement units %d is above the maximum allowed %d.", __FUNCTIONW__, mouseMovementUnits, kMouseMovementUnitsMax);

        virtualMouseMovementContributionBySource[(unsigned int)axis][sourceIdentifier] = mouseMovementUnits;
    }
}


namespace Xidi
{
    namespace Mouse
    {
        using namespace ::XidiTest;


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Mouse.h" for documentation.

        void SubmitMouseButtonPressedState(EMouseButton button)
        {
            std::scoped_lock lock(captureGuard);

            if (nullptr == capturingVirtualMouse)
                TEST_FAILED_BECAUSE(L"%s: No mock mouse is installed to capture a mouse button press event.", __FUNCTIONW__);

            capturingVirtualMouse->SubmitMouseButtonPressedState(button);
        }

        // --------

        void SubmitMouseButtonReleasedState(EMouseButton button)
        {
            std::scoped_lock lock(captureGuard);

            if (nullptr == capturingVirtualMouse)
                TEST_FAILED_BECAUSE(L"%s: No mock mouse is installed to capture a mouse button release event.", __FUNCTIONW__);

            capturingVirtualMouse->SubmitMouseButtonReleasedState(button);
        }

        // --------

        void SubmitMouseMovement(EMouseAxis axis, int mouseMovementUnits, uint32_t sourceIdentifier)
        {
            std::scoped_lock lock(captureGuard);

            if (nullptr == capturingVirtualMouse)
                TEST_FAILED_BECAUSE(L"%s: No mock mouse is installed to capture a mouse movement event.", __FUNCTIONW__);

            capturingVirtualMouse->SubmitMouseMovement(axis, mouseMovementUnits, sourceIdentifier);
        }
    }
}
