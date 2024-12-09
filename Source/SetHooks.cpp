/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file SetHooksWinMM.cpp
 *   Implementation of support functionality for setting hooks.
 **************************************************************************************************/

#include "SetHooks.h"

#include <Hookshot/Hookshot.h>
#include <Infra/Core/Message.h>

namespace Xidi
{
  void OutputSetHookResult(const wchar_t* functionName, Hookshot::EResult setHookResult)
  {
    if (Hookshot::SuccessfulResult(setHookResult))
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Info, L"Successfully set hook for %s.", functionName);
    else
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Failed (Hookshot::EResult = %u) to set hook for %s.",
          (unsigned int)setHookResult,
          functionName);
  }
} // namespace Xidi
