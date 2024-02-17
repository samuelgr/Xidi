/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file Utilities.h
 *   Declaration of test utility functions.
 **************************************************************************************************/

#pragma once

#include <sal.h>

namespace XidiTest
{
  /// Prints the specified message and appends a newline.
  /// @param [in] str Message string.
  void Print(const wchar_t* const str);

  /// Formats and prints the specified message.
  /// @param [in] format Message string, possibly with format specifiers.
  void PrintFormatted(_Printf_format_string_ const wchar_t* const format, ...);
} // namespace XidiTest
