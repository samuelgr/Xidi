/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file VirtualController.h
 *   Declaration of a complete virtual controller.
 *****************************************************************************/

#pragma once

#include "ControllerMapper.h"
#include "ControllerTypes.h"
#include "XInputInterface.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>


namespace Xidi
{
    /// Encapsulates all objects and provides all functionality needed by a complete virtual controller.
    /// Obtains state input from XInput, maps XInput data to virtual controller data, and applies transforms based on application-specified properties.
    /// Supports both instantaneous state and buffered state change events. All methods are concurrency-safe unless otherwise specified.
    class VirtualController
    {
    public:
        // -------- CONSTANTS ---------------------------------------------- //

        /// Minimum allowed value for an axis deadzone property, per DirectInput documentation.
        static constexpr uint32_t kAxisDeadzoneMin = 0;

        /// Maximum allowed value for an axis deadzone property, per DirectInput documentation.
        static constexpr uint32_t kAxisDeadzoneMax = 10000;

        /// Default value for an axis deadzone property.
        static constexpr uint32_t kAxisDeadzoneDefault = 1000;

        /// Minimum allowed value for an axis saturation property, per DirectInput documentation.
        static constexpr uint32_t kAxisSaturationMin = 0;

        /// Maximum allowed value for an axis saturation property, per DirectInput documentation.
        static constexpr uint32_t kAxisSaturationMax = 10000;

        /// Default value for an axis saturation property.
        static constexpr uint32_t kAxisSaturationDefault = 10000;


        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Integer type used to identify a controller to the underlying XInput interface.
        typedef DWORD TControllerIdentifier;

        /// Properties of an individual axis.
        /// Default values are roughly taken from DirectInput and XInput documentation.
        /// See DirectInput documentation for the meaning of each individual field.
        struct SAxisProperties
        {
            uint32_t deadzone;                                              ///< Deadzone of the axis, expressed as a percentage of the physical range around its center point. Can be from 0 (no deadzone) to 10000 (100% of the physical range is dead).
            int32_t deadzoneRawCutoffPositive;                              ///< Highest raw analog value on the positive side of the axis that falls within the deadzone region. Values at or below this should report neutral.
            int32_t deadzoneRawCutoffNegative;                              ///< Lowest raw analog value on the negative side of the axis that fallw within the deadzone region. Values at or above this should report neutral.

            uint32_t saturation;                                            ///< Saturation point of the axis, expressed as a percentage of its physical range in both directions. Can be from 0 (entire axis is saturated) to 10000 (do not saturate at all).
            int32_t saturationRawCutoffPositive;                            ///< Lowest raw analog value on the positive side of the axis that faills within the saturation region. Values at or above this should report extreme.
            int32_t saturationRawCutoffNegative;                            ///< Minimum value in the range of raw analog values that falls within the deadzone. Values at or below this should report extreme.

            int32_t rangeMin;                                               ///< Minimum reportable value for the axis.
            int32_t rangeMax;                                               ///< Maximum reportable value for the axis.
            int32_t rangeNeutral;                                           ///< Neutral value for the axis.

            /// Sets the deadzone and ensures value consistency between fields, but otherwise performs no error checking.
            /// @param [in] newDeadzone New deadzone value.
            inline void SetDeadzone(uint32_t newDeadzone)
            {
                deadzone = newDeadzone;
                deadzoneRawCutoffPositive = Controller::kAnalogValueNeutral + (((Controller::kAnalogValueMax - Controller::kAnalogValueNeutral) * (int32_t)newDeadzone) / kAxisDeadzoneMax);
                deadzoneRawCutoffNegative = Controller::kAnalogValueNeutral - (((Controller::kAnalogValueNeutral - Controller::kAnalogValueMin) * (int32_t)newDeadzone) / kAxisDeadzoneMax);
            }

            /// Sets the range and ensures value consistency between fields, but otherwise performs no error checking.
            /// @param [in] newRangeMin New minimum range value.
            /// @param [in] newRangeMax New maximum range value.
            inline void SetRange(int32_t newRangeMin, int32_t newRangeMax)
            {
                rangeMin = newRangeMin;
                rangeMax = newRangeMax;
                rangeNeutral = ((newRangeMin + newRangeMax) / 2);
            }
            
            /// Sets the saturation and ensures value consistency between fields, but otherwise performs no error checking.
            /// @param [in] newSaturation New saturation value.
            inline void SetSaturation(uint32_t newSaturation)
            {
                saturation = newSaturation;
                saturationRawCutoffPositive = Controller::kAnalogValueNeutral + (((Controller::kAnalogValueMax - Controller::kAnalogValueNeutral) * (int32_t)newSaturation) / kAxisSaturationMax);
                saturationRawCutoffNegative = Controller::kAnalogValueNeutral - (((Controller::kAnalogValueNeutral - Controller::kAnalogValueMin) * (int32_t)newSaturation) / kAxisSaturationMax);
            }
            
            /// Default constructor.
            /// Initializes fields to appropriate default values.
            SAxisProperties(void)
            {
                SetDeadzone(kAxisDeadzoneDefault);
                SetRange(Controller::kAnalogValueMin, Controller::kAnalogValueMax);
                SetSaturation(kAxisSaturationDefault);
            }
            
            /// Simple check for equality by low-level memory comparison.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SAxisProperties& other) const
            {
                return (0 == memcmp(this, &other, sizeof(*this)));
            }
        };

        /// Properties that apply to the whole device.
        struct SDeviceProperties
        {
            // Nothing defined yet.

            /// Simple check for equality by low-level memory comparison.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SDeviceProperties& other) const
            {
                return (0 == memcmp(this, &other, sizeof(*this)));
            }
        };

        /// Complete properties data structure.
        /// Holds all per-element and device-wide properties.
        struct SProperties
        {
            SAxisProperties axis[(int)Controller::EAxis::Count];            ///< Axis properties, one element per possible axis.
            SDeviceProperties device;                                       ///< Device-wide properties.

            /// Simple check for equality by low-level memory comparison.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SProperties& other) const
            {
                return (0 == memcmp(this, &other, sizeof(*this)));
            }
        };

        /// Identifier for a state data packet.
        struct SStateIdentifier
        {
            uint32_t packetNumber;                                          ///< Packet number. XInput provides a new packet number whenever a change in controller state is detected.
            uint32_t errorCode;                                             ///< Error code from XInput. Can be used to stop updating controller state whenever there is an error such as an unplugged controller.
        };


    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Controller identifier to be used when communicating with the underlying real controller.
        const TControllerIdentifier kControllerIdentifier;
        
        /// Provides concurrency control to the data structures in this virtual controller.
        std::recursive_mutex controllerMutex;
        
        /// Mapper to use for filling a virtual controller state object based on an XInput controller state.
        /// Not owned by, and must outlive, this object. Since in general mappers are created as constants, this constraint is reasonable.
        const Controller::Mapper& mapper;
        
        /// All properties associated with this virtual controller.
        SProperties properties;

        /// State of the virtual controller as of the last refresh.
        Controller::SState state;

        /// Identifies the last data packet that was retrieved from a real XInput controller during a refresh operation.
        /// Used to detect if there have been any changes.
        SStateIdentifier stateIdentifier;

        /// Specifies if a state refresh is needed before obtaining data from the virtual controller.
        /// Whenever a refresh operation occurs this flag is turned off. Whenever a data-gathering operation occurs (via state snapshot or otherwise) this flag is turned on.
        bool stateRefreshNeeded;

        /// Interface through which all XInput-related functionality is accessed.
        const std::unique_ptr<IXInput> xinput;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor.
        /// Requires a complete set of metadata for describing the virtual controller to be created.
        inline VirtualController(TControllerIdentifier controllerId, const Controller::Mapper& mapper, std::unique_ptr<IXInput>&& xinput = std::make_unique<XInput>()) : kControllerIdentifier(controllerId), controllerMutex(), mapper(mapper), properties(), state(), stateIdentifier(), stateRefreshNeeded(true), xinput(std::move(xinput))
        {
            // Nothing to do here.
        }

        /// Copy constructor. Should never be invoked.
        VirtualController(const VirtualController& other) = delete;


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Modifies the contents of the specified controller state object by applying this virtual controller's properties.
        /// Primarily intended for internal use but exposed for testing purposes. Implementation is not concurrency-safe.
        /// @param [in,out] controllerState Controller state object to transform.
        void ApplyProperties(Controller::SState* controllerState) const;
        
        /// Retrieves and returns the capabilities of this virtual controller.
        /// Controller capabilities act as metadata that are used internally and can be presented to applications.
        /// @return Read-only capabilities data structure reference.
        inline const Controller::SCapabilities& GetCapabilities(void) const
        {
            return mapper.GetCapabilities();
        }

        /// Retrieves and returns the deadzone property of the specified axis.
        /// @param [in] axis Target axis.
        /// @return Deadzone value associated with the target axis.
        inline uint32_t GetAxisDeadzone(Controller::EAxis axis) const
        {
            return properties.axis[(int)axis].deadzone;
        }

        /// Retrieves and returns the range property of the specified axis.
        /// @param [in] axis Target axis.
        /// @return Pair of range values associated with the target axis. First is the minimum, and second is the maximum.
        inline std::pair<int32_t, int32_t> GetAxisRange(Controller::EAxis axis) const
        {
            return std::make_pair(properties.axis[(int)axis].rangeMin, properties.axis[(int)axis].rangeMax);
        }

        /// Retrieves and returns the saturation property of the specified axis.
        /// @param [in] axis Target axis.
        /// @return Saturation value associated with the target axis.
        inline uint32_t GetAxisSaturation(Controller::EAxis axis) const
        {
            return properties.axis[(int)axis].saturation;
        }
        
        /// Fills the specified virtual controller state buffer with the latest view of the state of this virtual controller.
        /// @param [out] controllerState Buffer to be filled with the current state of this virtual controller.
        void GetState(Controller::SState* controllerState);

        /// Refreshes the view of the state of this virtual controller by querying the real XInput controller.
        /// @return `true` if the state of the controller changed since last refresh, `false` otherwise.
        bool RefreshState(void);
        
        /// Sets the deadzone property for a single axis.
        /// @param [in] axis Target axis.
        /// @param [in] deadzone Desired deadzone value.
        /// @return `true` if the new deadzone value was successfully validated and set, `false` otherwise.
        bool SetAxisDeadzone(Controller::EAxis axis, uint32_t deadzone);

        /// Sets the range property for a single axis.
        /// @param [in] axis Target axis.
        /// @param [in] rangeMin Desired minimum range value.
        /// @param [in] rangeMax Desired maximum range value.
        /// @return `true` if the new range was successfully validated and set, `false` otherwise.
        bool SetAxisRange(Controller::EAxis axis, int32_t rangeMin, int32_t rangeMax);

        /// Sets the saturation property for a single axis.
        /// @param [in] axis Target axis.
        /// @param [in] saturation Desired saturation value.
        /// @return `true` if the new saturation value was successfully validated and set, `false` otherwise.
        bool SetAxisSaturation(Controller::EAxis axis, uint32_t saturation);

        /// Sets the deadzone property for all axes.
        /// @param [in] deadzone Desired deadzone value.
        /// @return `true` if the new deadzone value was successfully validated and set, `false` otherwise.
        bool SetAllAxisDeadzone(uint32_t deadzone);

        /// Sets the range property for all axes.
        /// @param [in] rangeMin Desired minimum range value.
        /// @param [in] rangeMax Desired maximum range value.
        /// @return `true` if the new range was successfully validated and set, `false` otherwise.
        bool SetAllAxisRange(int32_t rangeMin, int32_t rangeMax);

        /// Sets the saturation property for all axes.
        /// @param [in] saturation Desired saturation value.
        /// @return `true` if the new saturation value was successfully validated and set, `false` otherwise.
        bool SetAllAxisSaturation(uint32_t saturation);
    };
}
