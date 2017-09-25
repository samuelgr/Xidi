/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
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

const std::wstring Globals::kDInputLibraryName = _T("dinput.dll");

const std::wstring Globals::kDInput8LibraryName = _T("dinput8.dll");

const std::wstring Globals::kWinMMLibraryName = _T("winmm.dll");


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Globals.h" for documentation.

HINSTANCE Globals::gInstanceHandle = NULL;

std::wstring Globals::gOverrideImportDirectInput = _T("");

std::wstring Globals::gOverrideImportDirectInput8 = _T("");

std::wstring Globals::gOverrideImportWinMM = _T("");


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
        stringToFill += _T('\\');
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
    TCHAR systemDirectoryPath[kMaximumSystemDirectoryNameLength];
    GetSystemDirectory(systemDirectoryPath, kMaximumSystemDirectoryNameLength);

    stringToFill = systemDirectoryPath;
}
