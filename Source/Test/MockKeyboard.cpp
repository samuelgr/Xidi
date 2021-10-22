/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MockKeyboard.cpp
 *   Declaration of a mock version of the keyboard interface along with
 *   additional testing-specific functions.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "MockKeyboard.h"
#include "TestCase.h"

#include <mutex>


namespace XidiTest
{
    using namespace ::Xidi::Keyboard;


    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Holds the mock keyboard object that is capturing input from the keyboard interface functions.
    static MockKeyboard* capturingVirtualKeyboard = nullptr;

    /// For ensuring proper concurrency control over the virtual keyboard capture state.
    static std::mutex captureGuard;


    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "MockKeyboard.h" for documentation.

    MockKeyboard::~MockKeyboard(void)
    {
        if (this == capturingVirtualKeyboard)
            EndCapture();
    }


    // -------- INSTANCE METHODS ------------------------------------------- //

    void MockKeyboard::BeginCapture(void)
    {
        std::scoped_lock lock(captureGuard);

        if (nullptr != capturingVirtualKeyboard)
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to attempting to replace another mock keyboard already capturing events.", __FUNCTIONW__);

        capturingVirtualKeyboard = this;
    }

    // --------

    void MockKeyboard::EndCapture(void)
    {
        std::scoped_lock lock(captureGuard);

        if (this != capturingVirtualKeyboard)
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to attempting to end capture for a mock keyboard not currently capturing events.", __FUNCTIONW__);

        capturingVirtualKeyboard = nullptr;
    }

    // --------

    void MockKeyboard::SubmitKeyPressedState(TKeyIdentifier key)
    {
        if (key >= virtualKeyboardState.max_size())
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to out-of-bounds controller identifier.", __FUNCTIONW__);

        virtualKeyboardState.insert(key);
    }

    // --------

    void MockKeyboard::SubmitKeyReleasedState(TKeyIdentifier key)
    {
        if (key >= virtualKeyboardState.max_size())
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to out-of-bounds controller identifier.", __FUNCTIONW__);

        virtualKeyboardState.erase(key);
    }
}


namespace Xidi
{
    namespace Keyboard
    {
        using namespace ::XidiTest;


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Keyboard.h" for documentation.

        void SubmitKeyPressedState(TKeyIdentifier key)
        {
            std::scoped_lock lock(captureGuard);

            if (nullptr == capturingVirtualKeyboard)
                TEST_FAILED_BECAUSE(L"%s: No mock keyboard is installed to capture a key press event.", __FUNCTIONW__);

            capturingVirtualKeyboard->SubmitKeyPressedState(key);
        }

        // --------

        void SubmitKeyReleasedState(TKeyIdentifier key)
        {
            std::scoped_lock lock(captureGuard);

            if (nullptr == capturingVirtualKeyboard)
                TEST_FAILED_BECAUSE(L"%s: No mock keyboard is installed to capture a key release event.", __FUNCTIONW__);

            capturingVirtualKeyboard->SubmitKeyReleasedState(key);
        }
    }
}
