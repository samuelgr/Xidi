/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Globals.cpp
 *   Implementation of accessors and mutators for global data items.
 *   Intended for miscellaneous data elements with no other suitable place.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"

#include <string>

using namespace Xidi;


// -------- CONSTANTS ------------------------------------------------------ //
// See "Globals.h" for documentation.

const std::wstring Globals::kDInputLibraryName = L"dinput.dll";

const std::wstring Globals::kDInput8LibraryName = L"dinput8.dll";

const std::wstring Globals::kWinMMLibraryName = L"winmm.dll";


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Globals.h" for documentation.

HINSTANCE Globals::gInstanceHandle = nullptr;

std::wstring Globals::gOverrideImportDirectInput = L"";

std::wstring Globals::gOverrideImportDirectInput8 = L"";

std::wstring Globals::gOverrideImportWinMM = L"";


// -------- CLASS METHODS -------------------------------------------------- //
// See "Globals.h" for documentation.

bool Globals::ApplyOverrideImportDirectInput(std::wstring& value)
{
    bool validValue = !(value.empty());

    if (true == validValue)
        gOverrideImportDirectInput = value;

    return validValue;
}

// --------

bool Globals::ApplyOverrideImportDirectInput8(std::wstring& value)
{
    bool validValue = !(value.empty());

    if (true == validValue)
        gOverrideImportDirectInput8 = value;

    return validValue;
}

// --------

bool Globals::ApplyOverrideImportWinMM(std::wstring& value)
{
    bool validValue = !(value.empty());

    if (true == validValue)
        gOverrideImportWinMM = value;

    return validValue;
}

// --------

void Globals::FillDirectInputLibraryPath(std::wstring& stringToFill)
{
    FillLibraryPath(stringToFill, gOverrideImportDirectInput, kDInputLibraryName);
}

// --------

void Globals::FillDirectInput8LibraryPath(std::wstring& stringToFill)
{
    FillLibraryPath(stringToFill, gOverrideImportDirectInput8, kDInput8LibraryName);
}

// --------

void Globals::FillWinMMLibraryPath(std::wstring& stringToFill)
{
    FillLibraryPath(stringToFill, gOverrideImportWinMM, kWinMMLibraryName);
}

// --------

HINSTANCE Globals::GetInstanceHandle(void)
{
    return gInstanceHandle;
}

// ---------

void Globals::SetInstanceHandle(HINSTANCE newInstanceHandle)
{
    gInstanceHandle = newInstanceHandle;
}


// -------- HELPERS -------------------------------------------------------- //
// See "Globals.h" for documentation.

void Globals::FillLibraryPath(std::wstring& stringToFill, const std::wstring& overridePath, const std::wstring& defaultLibraryFileName)
{
    if (overridePath.empty())
    {
        // No override path specified, so use the system directory plus library filename.
        FillSystemDirectoryPath(stringToFill);
        stringToFill += L'\\';
        stringToFill += defaultLibraryFileName;
    }
    else
    {
        // Override path specified, so just use that.
        stringToFill = overridePath;
    }
}

// --------

void Globals::FillSystemDirectoryPath(std::wstring& stringToFill)
{
    wchar_t systemDirectoryPath[kMaximumSystemDirectoryNameLength];
    GetSystemDirectory(systemDirectoryPath, kMaximumSystemDirectoryNameLength);

    stringToFill = systemDirectoryPath;
}
