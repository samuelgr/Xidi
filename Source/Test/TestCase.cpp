/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file TestCase.cpp
 *   Implementation of the test case interface.
 *****************************************************************************/

#include "Harness.h"
#include "TestCase.h"

#include <cstdarg>


namespace XidiTest
{
    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "TestCase.h" for documentation.

    ITestCase::ITestCase(const wchar_t* const name)
    {
        Harness::RegisterTestCase(this, name);
    }
}
