/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ControllerMapper.h
 *   Declaration of functionality used to implement mappings of an entire
 *   XInput controller layout to a virtual controller layout.
 *****************************************************************************/

#pragma once

#include "ControllerElementMapper.h"
#include "ControllerTypes.h"

#include <string_view>
#include <xinput.h>


namespace Xidi
{
    namespace Controller
    {
        /// Maps an XInput controller layout to a virtual controller layout.
        /// Each instance of this class represents a different virtual controller layout.
        class Mapper
        {
        public:
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// XInput controller element mappers, one per controller element.
            /// For controller elements that are not used, a value of `nullptr` may be used instead.
            struct SElementMap
            {
                const IElementMapper* stickLeftX = nullptr;
                const IElementMapper* stickLeftY = nullptr;
                const IElementMapper* stickRightX = nullptr;
                const IElementMapper* stickRightY = nullptr;
                const IElementMapper* dpadUp = nullptr;
                const IElementMapper* dpadDown = nullptr;
                const IElementMapper* dpadLeft = nullptr;
                const IElementMapper* dpadRight = nullptr;
                const IElementMapper* triggerLT = nullptr;
                const IElementMapper* triggerRT = nullptr;
                const IElementMapper* buttonA = nullptr;
                const IElementMapper* buttonB = nullptr;
                const IElementMapper* buttonX = nullptr;
                const IElementMapper* buttonY = nullptr;
                const IElementMapper* buttonLB = nullptr;
                const IElementMapper* buttonRB = nullptr;
                const IElementMapper* buttonBack = nullptr;
                const IElementMapper* buttonStart = nullptr;
                const IElementMapper* buttonLS = nullptr;
                const IElementMapper* buttonRS = nullptr;
            };

            /// Dual representation of a controller element map. Intended for internal use only.
            /// In one representation the elements all have names for element-specific access.
            /// In the other, all the elements are collapsed into an array for easy iteration.
            union UElementMap
            {
                SElementMap named;
                const IElementMapper* all[sizeof(SElementMap) / sizeof(const IElementMapper*)];

                inline UElementMap(const SElementMap& named) : named(named) {}
            };

        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Capabilities of the controller described by the element mappers in aggregate.
            const SCapabilities capabilities;

            /// All controller element mappers.
            const UElementMap elements;

            /// Name of this mapper.
            const std::wstring_view name;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor.
            /// Requires a name and, for each controller element, a unique element mapper which becomes owned by this object.
            /// For controller elements that are not used, `nullptr` may be passed instead.
            Mapper(const std::wstring_view name, const SElementMap& elements);

            /// Initialization constructor.
            /// Does not require or register a name for this mapper. This version is primarily useful for testing.
            /// Requires that a unique mapper be specified for each controller element, which in turn becomes owned by this object.
            /// For controller elements that are not used, `nullptr` may be passed instead.
            Mapper(const SElementMap& elements);

            /// Copy constructor. Should never be invoked.
            Mapper(const Mapper& other) = delete;
            
            /// Default destructor.
            ~Mapper(void);


            // -------- CLASS METHODS -------------------------------------- //

            /// Retrieves and returns a pointer to the mapper object whose type is specified.
            /// Mapper objects are created and managed internally, so this operation does not dynamically allocate or deallocate memory, nor should the caller attempt to free the returned pointer.
            /// If the default empty string value is used, then the mapper type that is retrieved is of default type.
            /// @param [in] mapperName Name of the desired mapper type. Supported values are defined in "ControllerMapper.cpp" as mapper instances.
            /// @return Pointer to the mapper of specified type, or `nullptr` if said type is unavailable.
            static const Mapper* GetByName(std::wstring_view mapperName = L"");


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Retrieves and returns the capabilities of the virtual controller layout implemented by the mapper.
            /// Controller capabilities act as metadata that are used internally and can be presented to applications.
            /// @return Read-only capabilities data structure reference.
            inline const SCapabilities& GetCapabilities(void) const
            {
                return capabilities;
            }

            /// Retrieves and returns the name of this mapper.
            /// @return Mapper name.
            inline const std::wstring_view GetName(void) const
            {
                return name;
            }

            /// Initializes and fills in the specified virtual controller state data structure object using the specified XInput controller state information.
            /// Does not apply any properties configured by the application, such as deadzone and range. All values produced use standard XInput settings.
            /// @param [out] controllerState Controller state object to be filled.
            /// @param [in] xinputState XInput controller state from which to read.
            void MapXInputState(SState* controllerState, const XINPUT_GAMEPAD& xinputState) const;
        };
    }
}
