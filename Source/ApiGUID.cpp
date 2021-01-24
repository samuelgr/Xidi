/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ApiGUID.cpp
 *   Implementation of helpers for integrating GUID types into the standard
 *   template library.
 *****************************************************************************/

#pragma once

#include "ApiGUID.h"

#include <cstddef>
#include <cstdint>
#include <functional>


// -------- OPERATORS ------------------------------------------------------ //
// See "ApiGUID.h" for documentation.

size_t std::hash<GUID>::operator()(REFGUID keyval) const
{
    static_assert(0 == (sizeof(GUID) % sizeof(size_t)), L"GUID size misalignment.");
    constexpr int kNumPieces = sizeof(GUID) / sizeof(size_t);

    const size_t* rawGUID = (const size_t*)&keyval;
    std::hash<size_t> hasher;

    size_t hash = 0;
    for (int i = 0; i < kNumPieces; ++i)
        hash ^= hasher(rawGUID[i]);

    return hash;
}

// --------

bool std::equal_to<GUID>::operator()(REFGUID lhs, REFGUID rhs) const
{
    return (memcmp(&lhs, &rhs, sizeof(lhs)) == 0);
}

// --------

bool std::less<GUID>::operator()(REFGUID lhs, REFGUID rhs) const
{
    return (memcmp(&lhs, &rhs, sizeof(lhs)) < 0);
}


// -------- EXPLICIT TEMPLATE INSTANTIATION -------------------------------- //

template struct std::hash<GUID>;
template struct std::equal_to<GUID>;
template struct std::less<GUID>;
