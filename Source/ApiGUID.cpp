/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ApiGUID.cpp
 *   Implementation of helpers for integrating GUID types into the standard template library.
 **************************************************************************************************/

#pragma once

#include "ApiGUID.h"

#include <cstddef>
#include <cstdint>
#include <functional>

size_t std::hash<GUID>::operator()(REFGUID keyval) const
{
  static_assert(
      0 == (sizeof(GUID) % sizeof(size_t)), "GUID size is not aligned with the piece size.");

  constexpr int kNumPieces = sizeof(GUID) / sizeof(size_t);
  static_assert(kNumPieces >= 1, "GUID size is too small compared to the piece size.");

  const size_t* rawGUID = (const size_t*)&keyval;
  std::hash<size_t> hasher;

  size_t hash = 0;
  for (int i = 0; i < kNumPieces; ++i)
    hash ^= hasher(rawGUID[i]);

  return hash;
}

bool std::equal_to<GUID>::operator()(REFGUID lhs, REFGUID rhs) const
{
  return (memcmp(&lhs, &rhs, sizeof(GUID)) == 0);
}

bool std::less<GUID>::operator()(REFGUID lhs, REFGUID rhs) const
{
  return (memcmp(&lhs, &rhs, sizeof(GUID)) < 0);
}

template struct std::hash<GUID>;
template struct std::equal_to<GUID>;
template struct std::less<GUID>;
