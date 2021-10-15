/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperParser.cpp
 *   Implementation of functionality for parsing pieces of mapper objects
 *   from strings, typically supplied in a configuration file.
 *****************************************************************************/


#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"
#include "MapperParser.h"

#include <map>
#include <optional>
#include <string_view>


namespace Xidi
{
    namespace Controller
    {
        namespace MapperParser
        {
            // -------- INTERNAL CONSTANTS --------------------------------- //

            /// Map of strings representing XInput controller elements to indices within the element map data structure.
            /// One pair exists per field in #SElementMap.
            static const std::map<std::wstring_view, unsigned int> kControllerElementStrings = {
                {L"StickLeftX", ELEMENT_MAP_INDEX_OF(stickLeftX)},
                {L"StickLeftY", ELEMENT_MAP_INDEX_OF(stickLeftY)},
                {L"StickRightX", ELEMENT_MAP_INDEX_OF(stickRightX)},
                {L"StickRightY", ELEMENT_MAP_INDEX_OF(stickRightY)},
                {L"DpadUp", ELEMENT_MAP_INDEX_OF(dpadUp)},
                {L"DpadDown", ELEMENT_MAP_INDEX_OF(dpadDown)},
                {L"DpadLeft", ELEMENT_MAP_INDEX_OF(dpadLeft)},
                {L"DpadRight", ELEMENT_MAP_INDEX_OF(dpadRight)},
                {L"TriggerLT", ELEMENT_MAP_INDEX_OF(triggerLT)},
                {L"TriggerRT", ELEMENT_MAP_INDEX_OF(triggerRT)},
                {L"ButtonA", ELEMENT_MAP_INDEX_OF(buttonA)},
                {L"ButtonB", ELEMENT_MAP_INDEX_OF(buttonB)},
                {L"ButtonX", ELEMENT_MAP_INDEX_OF(buttonX)},
                {L"ButtonY", ELEMENT_MAP_INDEX_OF(buttonY)},
                {L"ButtonLB", ELEMENT_MAP_INDEX_OF(buttonLB)},
                {L"ButtonRB", ELEMENT_MAP_INDEX_OF(buttonRB)},
                {L"ButtonBack", ELEMENT_MAP_INDEX_OF(buttonBack)},
                {L"ButtonStart", ELEMENT_MAP_INDEX_OF(buttonStart)},
                {L"ButtonLS", ELEMENT_MAP_INDEX_OF(buttonLS)},
                {L"ButtonRS", ELEMENT_MAP_INDEX_OF(buttonRS)}
            };


            // -------- INTERNAL FUNCTIONS --------------------------------- //

            //


            // -------- FUNCTIONS ------------------------------------------ //
            // See "MapperParser.h" for documentation.

            std::optional<unsigned int> FindControllerElementIndex(std::wstring_view controllerElementString)
            {
                const auto controllerElementIter = kControllerElementStrings.find(controllerElementString);

                if (kControllerElementStrings.cend() == controllerElementIter)
                    return std::nullopt;
                else
                    return controllerElementIter->second;
            }

            // --------

            bool IsControllerElementStringValid(std::wstring_view element)
            {
                return FindControllerElementIndex(element).has_value();
            }
        }
    }
}
