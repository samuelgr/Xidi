/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * Globals.cpp
 *      Implementation of accessors and mutators for global data items.
 *      Intended for miscellaneous data elements with no other suitable place.
 *****************************************************************************/

#include "ApiStdString.h"
#include "ApiWindows.h"
#include "Globals.h"

using namespace Xidi;


// -------- CONSTANTS ------------------------------------------------------ //
// See "Globals.h" for documentation.

const StdString Globals::kDInputLibraryName = _T("dinput.dll");

const StdString Globals::kDInput8LibraryName = _T("dinput8.dll");

const StdString Globals::kWinMMLibraryName = _T("winmm.dll");


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Globals.h" for documentation.

HINSTANCE Globals::gInstanceHandle = NULL;

StdString Globals::gOverrideImportDirectInput = _T("");

StdString Globals::gOverrideImportDirectInput8 = _T("");

StdString Globals::gOverrideImportWinMM = _T("");


// -------- CLASS METHODS -------------------------------------------------- //
// See "Globals.h" for documentation.

bool Globals::ApplyOverrideImportDirectInput(StdString& value)
{
    bool validValue = !(value.empty());

    if (true == validValue)
        gOverrideImportDirectInput = value;
    
    return validValue;
}

// --------

bool Globals::ApplyOverrideImportDirectInput8(StdString& value)
{
    bool validValue = !(value.empty());

    if (true == validValue)
        gOverrideImportDirectInput8 = value;

    return validValue;
}

// --------

bool Globals::ApplyOverrideImportWinMM(StdString& value)
{
    bool validValue = !(value.empty());

    if (true == validValue)
        gOverrideImportWinMM = value;

    return validValue;
}

// --------

void Globals::FillDirectInputLibraryPath(StdString& stringToFill)
{
    FillLibraryPath(stringToFill, gOverrideImportDirectInput, kDInputLibraryName);
}

// --------

void Globals::FillDirectInput8LibraryPath(StdString& stringToFill)
{
    FillLibraryPath(stringToFill, gOverrideImportDirectInput8, kDInput8LibraryName);
}

// --------

void Globals::FillWinMMLibraryPath(StdString& stringToFill)
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

void Globals::FillLibraryPath(StdString& stringToFill, const StdString& overridePath, const StdString& defaultLibraryFileName)
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

void Globals::FillSystemDirectoryPath(StdString& stringToFill)
{
    TCHAR systemDirectoryPath[kMaximumSystemDirectoryNameLength];
    GetSystemDirectory(systemDirectoryPath, kMaximumSystemDirectoryNameLength);

    stringToFill = systemDirectoryPath;
}
