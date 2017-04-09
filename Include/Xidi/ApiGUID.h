/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file ApiGUID.h
 *   Declaration of helpers for integrating GUID types into the standard
 *   template library.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ApiWindows.h"

#include <cstddef>
#include <functional>


// -------- HELPERS -------------------------------------------------------- //

// Used to produce hashes of GUID types.
template <> struct std::hash<GUID>
{
    size_t operator()(REFGUID keyval) const;
};

extern template struct std::hash<GUID>;


// Used to compare GUID types for equality.
template <> struct std::equal_to<GUID>
{
    bool operator()(REFGUID first, REFGUID second) const;
};

extern template struct std::equal_to<GUID>;
