/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MockKeyboard.cpp
 *   Implementation of a mock version of the keyboard interface along with additional
 *   testing-specific functions.
 **************************************************************************************************/

#include "MockKeyboard.h"

#include <mutex>

#include <Infra/Test/TestCase.h>

#include "ControllerTypes.h"

namespace XidiTest
{
  using namespace ::Xidi::Keyboard;

  /// Holds the mock keyboard object that is capturing input from the keyboard interface functions.
  static MockKeyboard* capturingVirtualKeyboard = nullptr;

  /// For ensuring proper concurrency control over the virtual keyboard capture state.
  static std::mutex captureGuard;

  MockKeyboard::~MockKeyboard(void)
  {
    if (this == capturingVirtualKeyboard) EndCapture();
  }

  void MockKeyboard::BeginCapture(void)
  {
    std::scoped_lock lock(captureGuard);

    if (nullptr != capturingVirtualKeyboard)
      TEST_FAILED_BECAUSE(
          L"%s: Test implementation error due to attempting to replace another mock keyboard already capturing events.",
          __FUNCTIONW__);

    capturingVirtualKeyboard = this;
  }

  void MockKeyboard::EndCapture(void)
  {
    std::scoped_lock lock(captureGuard);

    if (this != capturingVirtualKeyboard)
      TEST_FAILED_BECAUSE(
          L"%s: Test implementation error due to attempting to end capture for a mock keyboard not currently capturing events.",
          __FUNCTIONW__);

    capturingVirtualKeyboard = nullptr;
  }

  void MockKeyboard::SubmitKeyPressedState(TKeyIdentifier key)
  {
    if (key >= virtualKeyboardState.max_size())
      TEST_FAILED_BECAUSE(
          L"%s: Test implementation error due to out-of-bounds keyboard key identifier.",
          __FUNCTIONW__);

    virtualKeyboardState.insert(key);
  }

  void MockKeyboard::SubmitKeyReleasedState(TKeyIdentifier key)
  {
    if (key >= virtualKeyboardState.max_size())
      TEST_FAILED_BECAUSE(
          L"%s: Test implementation error due to out-of-bounds keyboard key identifier.",
          __FUNCTIONW__);

    virtualKeyboardState.erase(key);
  }
} // namespace XidiTest

namespace Xidi
{
  namespace Keyboard
  {
    using namespace ::XidiTest;

    void SubmitKeyPressedState(TKeyIdentifier key)
    {
      std::scoped_lock lock(captureGuard);

      if (nullptr == capturingVirtualKeyboard)
        TEST_FAILED_BECAUSE(
            L"%s: No mock keyboard is installed to capture a key press event.", __FUNCTIONW__);

      capturingVirtualKeyboard->SubmitKeyPressedState(key);
    }

    void SubmitKeyReleasedState(TKeyIdentifier key)
    {
      std::scoped_lock lock(captureGuard);

      if (nullptr == capturingVirtualKeyboard)
        TEST_FAILED_BECAUSE(
            L"%s: No mock keyboard is installed to capture a key release event.", __FUNCTIONW__);

      capturingVirtualKeyboard->SubmitKeyReleasedState(key);
    }
  } // namespace Keyboard
} // namespace Xidi
