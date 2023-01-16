/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file TestCase.cpp
 *   Implementation of the test case interface.
 *****************************************************************************/

#include "Harness.h"
#include "TestCase.h"

#include <string_view>


namespace XidiTest
{
    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "TestCase.h" for documentation.

    ITestCase::ITestCase(std::wstring_view name)
    {
        Harness::RegisterTestCase(this, name);
    }
}
