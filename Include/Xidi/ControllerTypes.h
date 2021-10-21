/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ControllerTypes.h
 *   Declaration of constants and types used for representing virtual
 *   controllers and their state.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <xinput.h>


namespace Xidi
{
    namespace Controller
    {
        // -------- CONSTANTS ---------------------------------------------- //

        /// Number of physical controllers that the underlying system supports.
        /// Not all will necessarily be physically present at any given time.
        /// Maximum allowable controller identifier is one less than this value.
        inline constexpr DWORD kPhysicalControllerCount = XUSER_MAX_COUNT;

        /// Maximum possible reading from an XInput controller's analog stick.
        /// Value taken from XInput documentation.
        inline constexpr int32_t kAnalogValueMax = 32767;

        /// Minimum possible reading from an XInput controller's analog stick.
        /// Value derived from the above to ensure symmetry around 0.
        /// This is slightly different than the XInput API itself, which allows negative values all the way down to -32768.
        inline constexpr int32_t kAnalogValueMin = -kAnalogValueMax;

        /// Neutral value for an XInput controller's analog stick.
        /// Value computed from extreme value constants above.
        inline constexpr int32_t kAnalogValueNeutral = (kAnalogValueMax + kAnalogValueMin) / 2;

        /// Maximum possible reading from an XInput controller's trigger.
        /// Value taken from XInput documentation.
        inline constexpr int32_t kTriggerValueMax = 255;

        /// Maximum possible reading from an XInput controller's trigger.
        /// Value taken from XInput documentation.
        inline constexpr int32_t kTriggerValueMin = 0;

        /// Midpoint reading from an XInput controller's trigger.
        inline constexpr int32_t kTriggerValueMid = (kTriggerValueMax + kTriggerValueMin) / 2;


        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Integer type used to identify physical controllers to the underlying system interfaces.
        typedef std::remove_const_t<decltype(kPhysicalControllerCount)> TControllerIdentifier;

        /// Enumerates all supported axis types using DirectInput terminology.
        /// It is not necessarily the case that all of these axes are present in a virtual controller. This enumerator just lists all the possible axes.
        /// Semantically, the value of each enumerator maps to an array position in the controller's internal state data structure.
        enum class EAxis : uint8_t
        {
            X,                                                              ///< X axis
            Y,                                                              ///< Y axis
            Z,                                                              ///< Z axis
            RotX,                                                           ///< X axis rotation
            RotY,                                                           ///< Y axis rotation
            RotZ,                                                           ///< Z axis rotation
            Count                                                           ///< Sentinel value, total number of enumerators
        };

        /// Enumerates all supported buttons.
        /// It is not necessarily the case that all of these buttons are present in a virtual controller. This enumerator just lists all the possible buttons.
        /// Semantically, the value of each enumerator maps to an array position in the controller's internal state data structure.
        enum class EButton : uint8_t
        {
            B1,                                                             ///< Button 1
            B2,                                                             ///< Button 2
            B3,                                                             ///< Button 3
            B4,                                                             ///< Button 4
            B5,                                                             ///< Button 5
            B6,                                                             ///< Button 6
            B7,                                                             ///< Button 7
            B8,                                                             ///< Button 8
            B9,                                                             ///< Button 9
            B10,                                                            ///< Button 10
            B11,                                                            ///< Button 11
            B12,                                                            ///< Button 12
            B13,                                                            ///< Button 13
            B14,                                                            ///< Button 14
            B15,                                                            ///< Button 15
            B16,                                                            ///< Button 16
            Count                                                           ///< Sentinel value, total number of enumerators
        };

        /// Enumerates buttons that correspond to each of the possible POV directions.
        /// Xidi either presents, or does not present, a POV to the application.
        /// If a POV is presented, then these four buttons in the internal state data structure are combined into a POV reading.
        /// If not, then the corresponding part of the internal state data structure is ignored.
        /// Semantically, the value of each enumerator maps to an array position in the controller's internal state data structure.
        enum class EPovDirection : uint8_t
        {
            Up,                                                             ///< Up direction
            Down,                                                           ///< Down direction
            Left,                                                           ///< Left direction
            Right,                                                          ///< Right direction
            Count                                                           ///< Sentinel value, total number of enumerators
        };

        /// Enumerates all types of controller elements present in the internal virtual controller state.
        /// The special whole controller value indicates that a reference is being made to the entire virtual controller rather than any specific element.
        enum class EElementType : uint8_t
        {
            Axis,
            Button,
            Pov,
            WholeController
        };

        /// Identifier for an element of a virtual controller's state.
        /// Specifies both element type and index. Valid member of the union is based on the indicated type.
        struct SElementIdentifier
        {
            EElementType type;
            
            union
            {
                EAxis axis;
                EButton button;
            };

            /// Simple check for equality by low-level memory comparison.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SElementIdentifier& other) const
            {
                return (0 == memcmp(this, &other, sizeof(*this)));
            }
        };
        static_assert(sizeof(SElementIdentifier) <= 4, "Data structure size constraint violation.");

        /// Capabilities of a Xidi virtual controller.
        /// Filled in by looking at a mapper and used during operations like EnumObjects to tell the application about the virtual controller's components.
        struct SCapabilities
        {
            EAxis axisType[(int)EAxis::Count];                              ///< Type of each axis present. When the controller is presented to the application, all the axes on it are presented with contiguous indices. This array is used to map from DirectInput axis index to internal axis index.
            struct
            {
                uint8_t numAxes : 3;                                        ///< Number of axes in the virtual controller, also the number of elements of the axis type array that are valid.
                uint8_t numButtons : 5;                                     ///< Number of buttons present in the virtual controller.
            };
            bool hasPov;                                                    ///< Specifies whether or not the virtual controller has a POV. If it does, then the POV buttons in the controller state are used, otherwise they are ignored.

            /// Simple check for equality by low-level memory comparison.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SCapabilities& other) const
            {
                return (0 == memcmp(this, &other, sizeof(*this)));
            }

            /// Appends an axis to the list of axis types in this capabilities object.
            /// Performs no bounds-checking or uniqueness-checking, so this is left to the caller to ensure.
            /// @param [in] axis Axis to append to the list of present axes.
            constexpr inline void AppendAxis(EAxis axis)
            {
                axisType[numAxes] = axis;
                numAxes += 1;
            }

            /// Determines the index of the specified axis type within this capabilities object, if it exists.
            /// @param [in] axis Axis type for which to query.
            /// @return Index of the specified axis if it is present, or -1 if it is not.
            constexpr inline int FindAxis(EAxis axis) const
            {
                for (int i = 0; i < numAxes; ++i)
                {
                    if (axisType[i] == axis)
                        return i;
                }

                return -1;
            }

            /// Checks if this capabilities object specifies that the controller has an axis of the specified type.
            /// @param [in] axis Axis type for which to query.
            /// @return `true` if the axis is present, `false` otherwise.
            constexpr inline bool HasAxis(EAxis axis) const
            {
                return (-1 != FindAxis(axis));
            }

            /// Checks if this capabilities object specifies that the controller has a button of the specified number.
            /// @param [in] button Button number for which to query.
            /// @return `true` if the button is present, `false` otherwise.
            constexpr inline bool HasButton(EButton button) const
            {
                return ((uint8_t)button < numButtons);
            }
        };
        static_assert(sizeof(SCapabilities) <= 8, "Data structure size constraint violation.");
        static_assert((uint8_t)EAxis::Count <= 0b111, "Number of axes does not fit into 3 bits.");
        static_assert((uint8_t)EButton::Count <= 0b11111, "Number of buttons does not fit into 5 bits.");

        /// Holds POV direction, which is presented both as an array of separate components and as a single aggregated integer view.
        union UPovDirection
        {
            bool components[(int)EPovDirection::Count];                     ///< Pressed (`true`) or unpressed (`false`) state for each POV direction separately, one element per button. Bitset versus boolean produces no size difference, given the number of POV directions.
            uint32_t all;                                                   ///< Aggregate state of all POV directions, available as a single quantity for easy comparison and assignment.
        };
        static_assert(sizeof(UPovDirection::components) == sizeof(UPovDirection::all), "Mismatch in POV view sizes.");
        
        /// Native data format for virtual controllers, used internally to represent controller state.
        /// Instances of `XINPUT_GAMEPAD` are passed through a mapper to produce objects of this type.
        /// Validity or invalidity of each element depends on the mapper.
        struct SState
        {
            int32_t axis[(int)EAxis::Count];                                ///< Values for all axes, one element per axis.
            std::bitset<(int)EButton::Count> button;                        ///< Pressed (`true`) or unpressed (`false`) state for each button, one bit per button. Bitset is used as a size optimization, given the number of buttons.
            UPovDirection povDirection;                                     ///< POV direction, presented simultaneously as individual components and as an aggregate quantity.

            /// Simple check for equality by low-level memory comparison.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SState& other) const
            {
                return (0 == memcmp(this, &other, sizeof(*this)));
            }
        };
        static_assert(sizeof(SState) <= 32, "Data structure size constraint violation.");
    }
}
