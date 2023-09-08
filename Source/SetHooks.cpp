/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file SetHooksWinMM.cpp
 *   Implementation of support functionality for setting hooks.
 **************************************************************************************************/

#include "SetHooks.h"

#include <Hookshot/Hookshot.h>

#include "Message.h"

namespace Xidi
{
  void OutputSetHookResult(const wchar_t* functionName, Hookshot::EResult setHookResult)
  {
    if (Hookshot::SuccessfulResult(setHookResult))
      Message::OutputFormatted(
          Message::ESeverity::Info, L"Successfully set hook for %s.", functionName);
    else
      Message::OutputFormatted(
          Message::ESeverity::Error,
          L"Failed (Hookshot::EResult = %u) to set hook for %s.",
          (unsigned int)setHookResult,
          functionName);
  }
} // namespace Xidi
