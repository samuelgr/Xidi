/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file TestCase.cpp
 *   Implementation of the test case interface.
 **************************************************************************************************/

#include "TestCase.h"

#include <string_view>

#include "Harness.h"

namespace XidiTest
{
  ITestCase::ITestCase(std::wstring_view name)
  {
    Harness::RegisterTestCase(this, name);
  }
} // namespace XidiTest
