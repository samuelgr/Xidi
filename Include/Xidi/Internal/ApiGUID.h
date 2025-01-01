/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ApiGUID.h
 *   Declaration of helpers for integrating GUID types into the standard template library.
 **************************************************************************************************/

#pragma once

#include <cstddef>
#include <functional>

#include "ApiWindows.h"

/// Used to produce hashes of GUID types.
template <> struct std::hash<GUID>
{
  size_t operator()(REFGUID keyval) const;
};

/// Used to compare GUID types for equality.
template <> struct std::equal_to<GUID>
{
  bool operator()(REFGUID lhs, REFGUID rhs) const;
};

/// Used to compare GUID types for ordering purposes.
template <> struct std::less<GUID>
{
  bool operator()(REFGUID lhs, REFGUID rhs) const;
};
