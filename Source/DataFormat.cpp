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
    // -------- CLASS METHODS ---------------------------------------------- //
    // See "DataFormat.h" for documentation.

    const DataFormat* DataFormat::CreateFromApplicationFormatSpec(const DIDATAFORMAT& appFormatSpec)
    {
        return nullptr;
    }

    // --------

    DataFormat::EPovValue DataFormat::PovDirectionFromControllerState(const Controller::SState& controllerState)
    {
        static constexpr DataFormat::EPovValue kPovDirectionValues[3][3] = {
            {DataFormat::EPovValue::NW, DataFormat::EPovValue::N,      DataFormat::EPovValue::NE},
            {DataFormat::EPovValue::W,  DataFormat::EPovValue::Center, DataFormat::EPovValue::E},
            {DataFormat::EPovValue::SW, DataFormat::EPovValue::S,      DataFormat::EPovValue::SE}
        };

        const int xCoord = ((true == controllerState.povDirection[(int)Controller::EPovDirection::Right]) ? 1 : 0) - ((true == controllerState.povDirection[(int)Controller::EPovDirection::Left]) ? 1 : 0);
        const int yCoord = ((true == controllerState.povDirection[(int)Controller::EPovDirection::Down]) ? 1 : 0) - ((true == controllerState.povDirection[(int)Controller::EPovDirection::Up]) ? 1 : 0);

        return kPovDirectionValues[1 + yCoord][1 + xCoord];
    }


    // -------- INSTANCE METHODS ------------------------------------------- //
    // See "DataFormat.h" for documentation.

    std::optional<Controller::SElementIdentifier> DataFormat::GetElementForOffset(TOffset offset)
    {
        const auto elementRecord = dataFormatSpec.offsetElementMap.find(offset);
        if (elementRecord != dataFormatSpec.offsetElementMap.cend())
            return elementRecord->second;

        return std::nullopt;
    }

    // --------

    std::optional<DataFormat::TOffset> DataFormat::GetOffsetForElement(Controller::SElementIdentifier element)
    {
        switch (element.type)
        {
        case Controller::EElementType::Axis:
            if (kInvalidOffsetValue != dataFormatSpec.axisOffset[(int)element.axis])
                return dataFormatSpec.axisOffset[(int)element.axis];
            break;

        case Controller::EElementType::Button:
            if (kInvalidOffsetValue != dataFormatSpec.buttonOffset[(int)element.button])
                return dataFormatSpec.buttonOffset[(int)element.button];
            break;

        case Controller::EElementType::Pov:
            if (kInvalidOffsetValue != dataFormatSpec.povOffset)
                return dataFormatSpec.povOffset;
            break;
        }
        
        return std::nullopt;
    }

    // --------

    std::optional<Controller::SElementIdentifier> DataFormat::IdentifyElement(DWORD dwObj, DWORD dwHow)
    {
        switch (dwHow)
        {
        case DIPH_DEVICE:
            // Whole device is referenced.
            // Per DirectInput documentation, the object identifier must be 0.
            if (0 == dwObj)
                return Controller::SElementIdentifier({.type = Controller::EElementType::WholeController});
            break;

        case DIPH_BYOFFSET:
            // Controller element is being identified by offset.
            // Object identifier is an offset into the application's data format.
            return GetElementForOffset(dwObj);

        case DIPH_BYID:
            // Controller element is being identified by instance identifier.
            // Object identifier contains type and index, and the latter refers to the controller's reported capabilities.
            if ((int)DIDFT_GETINSTANCE(dwObj) >= 0)
            {
                const int kType = (int)DIDFT_GETTYPE(dwObj);
                const int kIndex = (int)DIDFT_GETINSTANCE(dwObj);

                switch (kType)
                {
                case DIDFT_ABSAXIS:
                    if ((kIndex < (int)Controller::EAxis::Count) && (kIndex < controllerCapabilities.numAxes))
                        return Controller::SElementIdentifier({.type = Controller::EElementType::Axis, .axis = controllerCapabilities.axisType[kIndex]});
                    break;

                case DIDFT_PSHBUTTON:
                    if ((kIndex < (int)Controller::EButton::Count) && (kIndex < controllerCapabilities.numButtons))
                        return Controller::SElementIdentifier({.type = Controller::EElementType::Button, .button = (Controller::EButton)kIndex});
                    break;

                case DIDFT_POV:
                    if (kIndex == 0)
                        return Controller::SElementIdentifier({.type = Controller::EElementType::Pov});
                    break;
                }
            }
            break;
        }
        
        return std::nullopt;
    }

    // --------

    bool DataFormat::WriteDataPacket(void* packetBuffer, TOffset packetBufferSizeBytes, const Controller::SState& controllerState)
    {
        // Sanity check: did the application allocate sufficient buffer space?
        if (packetBufferSizeBytes < dataFormatSpec.packetSizeBytes)
            return false;

        uint8_t* const packetByteBuffer = (uint8_t*)packetBuffer;

        // Initialize the application data packet.
        // Everything not explicitly written will be 0, except for unused POVs which must be initialized to center position.
        ZeroMemory(packetBuffer, packetBufferSizeBytes);
        for (auto povOffsetUnused : dataFormatSpec.povOffsetsUnused)
        {
            EPovValue* const valueLocation = (EPovValue*)(&packetByteBuffer[povOffsetUnused]);
            *valueLocation = EPovValue::Center;
        }
        
        // Axis values
        for (int i = 0; i < _countof(dataFormatSpec.axisOffset); ++i)
        {
            if (kInvalidOffsetValue != dataFormatSpec.axisOffset[i])
            {
                TAxisValue* const valueLocation = (TAxisValue*)(&packetByteBuffer[dataFormatSpec.axisOffset[i]]);
                *valueLocation = (TAxisValue)controllerState.axis[i];
            }
        }

        // Button values
        for (int i = 0; i < _countof(dataFormatSpec.buttonOffset); ++i)
        {
            if (kInvalidOffsetValue != dataFormatSpec.buttonOffset[i])
            {
                TButtonValue* const valueLocation = (TButtonValue*)(&packetByteBuffer[dataFormatSpec.axisOffset[i]]);
                *valueLocation = ((true == controllerState.button[i]) ? kButtonValuePressed : kButtonValueNotPressed);
            }
        }

        // POV value
        if (kInvalidOffsetValue != dataFormatSpec.povOffset)
        {
            EPovValue* const valueLocation = (EPovValue*)(&packetByteBuffer[dataFormatSpec.povOffset]);
            *valueLocation = PovDirectionFromControllerState(controllerState);
        }
        
        return true;
    }
}
