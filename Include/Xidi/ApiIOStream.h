/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file ApiIOStream.h
 *   Common header file for specifying wide or narrow I/O stream objects.
 *****************************************************************************/

#pragma once

#include <iostream>


// -------- MACROS --------------------------------------------------------- //

// Helpers for iostream input and output when using Unicode.
#ifdef UNICODE
#define terr                                    wcerr
#define tin                                     wcin
#define tout                                    wcout
#else
#define terr                                    cerr
#define tin                                     cin
#define tout                                    cout
#endif
