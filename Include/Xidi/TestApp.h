/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * TestApp.h
 *      Common declarations and helpers for all versions of the test app.
 *****************************************************************************/

#pragma once

#include <cstdlib>
#include <iostream>

using namespace std;


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
