/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ApiGUID.cpp
 *   Implementation of helpers for integrating GUID types into the standard
 *   template library.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ApiGUID.h"

#include <cstddef>
#include <cstdint>
#include <functional>


// -------- HELPERS -------------------------------------------------------- //
// See "ApiGUID.h" for documentation.

size_t std::hash<GUID>::operator()(REFGUID keyval) const
{
    const size_t* rawGUID = (const size_t*)&keyval;
    std::hash<size_t> hasher;

    return hasher(rawGUID[0]) ^ (hasher(rawGUID[1]) << 1) ^ (hasher(rawGUID[2]) << 2) ^ (hasher(rawGUID[3]) << 3);
}

template struct std::hash<GUID>;

// --------

bool std::equal_to<GUID>::operator()(REFGUID first, REFGUID second) const
{
    const uint32_t* rawGUID1 = (const uint32_t*)&first;
    const uint32_t* rawGUID2 = (const uint32_t*)&second;

    return ((rawGUID1[0] == rawGUID2[0]) && (rawGUID1[1] == rawGUID2[1]) && (rawGUID1[2] == rawGUID2[2]) && (rawGUID1[3] == rawGUID2[3]));
}

template struct std::equal_to<GUID>;
