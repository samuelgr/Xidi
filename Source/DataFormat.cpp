/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file DataFormat.cpp
 *   Implementation of all functionality related to parsing and formatting
 *   controller data using the format specified by a DirectInput application.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "Message.h"

#include <map>
#include <optional>
#include <vector>


namespace Xidi
{
    namespace Controller
    {
        // -------- CLASS METHODS ------------------------------------------ //
        // See "DataFormat.h" for documentation.

        const DataFormat* DataFormat::CreateFromApplicationFormatSpec(const DIDATAFORMAT& appFormatSpec)
        {
            return nullptr;
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "DataFormat.h" for documentation.

        std::optional<SElementIdentifier> DataFormat::GetElementForOffset(TOffset offset)
        {
            return std::nullopt;
        }

        // --------

        std::optional<DataFormat::TOffset> DataFormat::GetOffsetForElement(SElementIdentifier element)
        {
            return std::nullopt;
        }

        // --------

        std::optional<SElementIdentifier> DataFormat::IdentifyElement(DWORD dwObj, DWORD dwHow)
        {
            return std::nullopt;
        }

        // --------

        bool DataFormat::WriteDataPacket(void* packetBuffer, TOffset packetBufferSizeBytes, const SState& controllerState)
        {
            return false;
        }
    }
}
