/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file ApiCharacterType.h
 *   Common header file for specifying wide or narrow character type APIs.
 *****************************************************************************/

#pragma once

#include <cctype>


// -------- MACROS --------------------------------------------------------- //

// Helpers for string type specification.
#ifdef UNICODE
#define istalnum                                iswalnum
#define istalpha                                iswalpha
#define istblank                                iswblank
#define istcntrl                                iswcntrl
#define istdigit                                iswdigit
#define istgraph                                iswgraph
#define istlower                                iswlower
#define istprint                                iswprint
#define istpunct                                iswpunct
#define istspace                                iswspace
#define istupper                                iswupper
#define istxdigit                               iswxdigit
#else
#define istalnum                                isalnum
#define istalpha                                isalpha
#define istblank                                isblank
#define istcntrl                                iscntrl
#define istdigit                                isdigit
#define istgraph                                isgraph
#define istlower                                islower
#define istprint                                isprint
#define istpunct                                ispunct
#define istspace                                isspace
#define istupper                                isupper
#define istxdigit                               isxdigit
#endif
