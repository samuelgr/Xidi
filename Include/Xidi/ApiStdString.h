/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * ApiStdString.h
 *      Common header file for specifying wide or narrow standard strings.
 *****************************************************************************/

#pragma once

#include <string>


// -------- TYPE DEFINITIONS ----------------------------------------------- //

// Helpers for string type specification.
#ifdef UNICODE
typedef std::wstring                            StdString;
#else
typedef std::string                             StdString;
#endif
