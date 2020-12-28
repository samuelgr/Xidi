/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ControllerTypes.h
 *   Declaration of constants and types used for representing virtual
 *   controllers and their state.
 *****************************************************************************/

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>


namespace Xidi
{
    namespace Controller
    {
        // -------- CONSTANTS ---------------------------------------------- //

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


        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Enumerates all supported axis types using DirectInput terminology.
        /// It is not necessarily the case that all of these axes are present in a virtual controller. This enumerator just lists all the possible axes.
        /// Semantically, the value of each enumerator maps to an array position in the controller's internal state data structure.
        enum class EAxis
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
        enum class EButton
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
        enum class EPov
        {
            Up,                                                             ///< Up direction
            Down,                                                           ///< Down direction
            Left,                                                           ///< Left direction
            Right,                                                          ///< Right direction
            Count                                                           ///< Sentinel value, total number of enumerators
        };

        /// Enumerates all types of controller elements present in the internal virtual controller state.
        enum class EElementType
        {
            Axis,
            Button,
            Pov
        };

        /// Properties of an individual axis.
        /// Default values are roughly taken from DirectInput and XInput documentation.
        /// See DirectInput documentation for the meaning of each individual field.
        struct SAxisProperties
        {
            uint32_t deadzone = 1000;                                       ///< Deadzone of the axis, expressed as a percentage of the physical range around its center point. Can be from 0 (no deadzone) to 10000 (100% of the physical range is dead).
            uint32_t saturation = 10000;                                    ///< Saturation point of the axis, expressed as a percentage of its physical range in both directions. Can be from 0 (entire axis is saturated) to 10000 (do not saturate at all).
            int32_t rangeMin = kAnalogValueMin;                             ///< Minimum reportable value for the axis.
            int32_t rangeMax = kAnalogValueMax;                             ///< Maximum reportable value for the axis.

            /// Simple check for equality by low-level memory comparison.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SAxisProperties& other) const
            {
                return (0 == memcmp(this, &other, sizeof(*this)));
            }
        };

        /// Capabilities of a Xidi virtual controller.
        /// Filled in by looking at a mapper and used during operations like EnumObjects to tell the application about the virtual controller's components.
        struct SCapabilities
        {
            EAxis axisType[(int)EAxis::Count];                              ///< Type of each axis present. When the controller is presented to the application, all the axes on it are presented with contiguous indices. This array is used to map from DirectInput axis index to internal axis index.
            int numAxes;                                                    ///< Number of axes in the virtual controller, also the number of elements of the axis type array that are valid.
            int numButtons;                                                 ///< Number of buttons present in the virtual controller.
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
            inline void AppendAxis(EAxis axis)
            {
                axisType[numAxes] = axis;
                numAxes += 1;
            }

            /// Determines the index of the specified axis type within this capabilities object, if it exists.
            /// @param [in] axis Axis type for which to query.
            /// @return Index of the specified axis if it is present, or -1 if it is not.
            inline int FindAxis(EAxis axis)
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
            inline bool HasAxis(EAxis axis)
            {
                return (-1 != FindAxis(axis));
            }
        };
        
        /// Native data format for virtual controllers, used internally to represent controller state.
        /// Instances of `XINPUT_GAMEPAD` are passed through a mapper to produce objects of this type.
        /// Validity or invalidity of each element depends on the mapper.
        struct SState
        {
            int32_t axis[(int)EAxis::Count];
            bool button[(int)EButton::Count];
            bool povDirection[(int)EPov::Count];

            /// Simple check for equality by low-level memory comparison.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SState& other) const
            {
                return (0 == memcmp(this, &other, sizeof(*this)));
            }
        };
    }
}
