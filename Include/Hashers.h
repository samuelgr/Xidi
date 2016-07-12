/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Hashers.h
 *      Provides support for hashing useful but non-standard key types.
 *****************************************************************************/

#pragma once

#include <guiddef.h>
#include <xstddef>


// -------- MACROS --------------------------------------------------------- //

// Provides a mechanism for hashing arbitrary types in a byte-wise manner by specifying the byte address and byte count of an item to be hashed.
// Simply uses Microsoft's internal mechanism for doing this.
#define BytewiseHash(key, count)                _Hash_seq((const unsigned char*)&(key), (count))


// -------- FUNCTIONS ------------------------------------------------------ //

// Hash function for GUIDs.
template<> struct std::hash<const GUID>
{
    size_t operator()(REFGUID key) const
    {
        return BytewiseHash(key, sizeof(GUID));
    }
};
