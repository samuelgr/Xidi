/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ElementMapper.h
 *   Declaration of functionality used to implement mappings from individual
 *   XInput controller elements to virtual DirectInput controller elements.
 *****************************************************************************/

#pragma once

#include "ControllerTypes.h"

#include <cstdint>
#include <optional>


namespace Xidi
{
    namespace Controller
    {
        /// Interface for mapping an XInput controller element's state reading to an internal controller state data structure value.
        /// An instance of this object exists for each XInput controller element in a mapper.
        class IElementMapper
        {
        public:
            // -------- ABSTRACT INSTANCE METHODS -------------------------- //

            /// Calculates the contribution to controller state from a given analog reading in the standard XInput axis range -32768 to +32767.
            /// Contribution is aggregated with anything that already exists in the controller state.
            /// @param [in,out] controllerState Controller state data structure to be updated.
            /// @param [in] analogStickValue Raw analog stick value from the XInput controller.
            virtual void ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const = 0;

            /// Calculates the contribution to controller state from a given button pressed status reading.
            /// Contribution is aggregated with anything that already exists in the controller state.
            /// @param [in,out] controllerState Controller state data structure to be updated.
            /// @param [in] buttonPressed Button state from the XInput controller, `true` if pressed and `false` otherwise.
            virtual void ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const = 0;

            /// Calculates the contribution to controller state from a given trigger reading in the standard XInput trigger range 0 to 255.
            /// Contribution is aggregated with anything that already exists in the controller state.
            /// @param [in,out] controllerState Controller state data structure to be updated.
            /// @param [in] buttonPressed Button state from the XInput controller, `true` if pressed and `false` otherwise.
            virtual void ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const = 0;

            /// Specifies the virtual controller element that is the target of any contributions from this element mapper.
            /// @return Identifier of the targert virtual controller element.
            virtual SElementIdentifier GetTargetElement(void) const = 0;
        };

        /// Maps a single XInput controller element such that it contributes to an axis value on a virtual controller.
        /// For analog sticks and triggers, the value read is mapped directly to the corresponding virtual controller axis. Half-axis mode generally makes sense only for triggers because they can share an axis, but it is implemented by range mapping for analog stick axes as well.
        /// For buttons, the value is either negative extreme if the button is not pressed or positive extreme if the value is pressed. Use a half-axis configuration to map to either neutral (not pressed) or extreme value (pressed).
        class AxisMapper : public IElementMapper
        {
        public:
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// Enumerates the possible directions to which the contributions of this element should be mapped.
            /// Typically an analog stick axis would contribute to the whole axis (i.e. both directions).
            /// Triggers might be made to share an axis by having one be positive and one be negative.
            enum class EDirection : uint8_t
            {
                Both,                                                       ///< Specifies that the contribution is to the whole axis, mapping evenly to both directions.
                Positive,                                                   ///< Specifies that the contribution is only to the positive part of the axis.
                Negative                                                    ///< Specifies that the contribution is only to the negative part of the axis.
            };


        protected:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Identifies the axis to which this mapper should contribute in the internal controller state data structure.
            const EAxis axis;

            /// Identifies the direction to which this mapper should contribute on its associated axis.
            /// If set to anything other than both directions, the contribution is to half of the axis only.
            const EDirection direction;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //
            
            /// Initialization constructor.
            /// Specifies the axis and, optionally, the direction to which this mapper should contribute in the internal controller state data structure.
            inline constexpr AxisMapper(EAxis axis, EDirection direction = EDirection::Both) : IElementMapper(), axis(axis), direction(direction)
            {
                // Nothing to do here.
            }


            // -------- CONCRETE INSTANCE METHODS -------------------------- //

            void ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const override;
            void ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const override;
            void ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const override;
            SElementIdentifier GetTargetElement(void) const override;
        };

        /// Maps a single XInput controller element such that it contributes to a button reading on a virtual controller.
        /// For analog sticks, if the axis displacement from neutral is greater than a threshold, the button is considered pressed.
        /// For triggers, if the magnitude of the trigger reading is greater than a threshold, the button is considered pressed.
        /// For buttons, the button state is mapped directly to the target button.
        class ButtonMapper : public IElementMapper
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Identifies the button to which this mapper should contribute in the internal controller state data structure.
            const EButton button;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor.
            /// Specifies the button to which all updates are contributed.
            inline constexpr ButtonMapper(EButton button) : IElementMapper(), button(button)
            {
                // Nothing to do here.
            }


            // -------- CONCRETE INSTANCE METHODS -------------------------- //

            void ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const override;
            void ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const override;
            void ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const override;
            SElementIdentifier GetTargetElement(void) const override;
        };

        /// Maps a single XInput controller element such that it contributes to an axis value on a virtual controller, but removes analog functionality. Values contributed are either zero or extreme.
        /// For analog sticks, the value read is mapped to either neutral or an extreme axis value. In whole-axis mode, the possible values are negative extreme, neutral, and positive extreme. In half-axis mode, possible values are neutral and extreme (input in the inactive direction is ignored).
        /// For triggers, which unlike analog sticks do not have a centered neutral position, possible values depend on the axis mode. In whole-axis mode, the possible values are negative extreme and positive extreme. In half-axis mode, the possible values are neutral and extreme.
        /// For buttons, the behavior is the same as the standard button-to-axis mapping behavior.
        class DigitalAxisMapper : public AxisMapper
        {
        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor.
            /// Specifies the axis and, optionally, the direction to which this mapper should contribute in the internal controller state data structure.
            inline constexpr DigitalAxisMapper(EAxis axis, EDirection direction = EDirection::Both) : AxisMapper(axis, direction)
            {
                // Nothing to do here.
            }


            // -------- CONCRETE INSTANCE METHODS -------------------------- //

            void ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const override;
            void ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const override;
        };

        /// Maps a single XInput controller element such that it contributes to a POV on a virtual controller.
        class PovMapper : public IElementMapper
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Identifies the POV direction to which this mapper should nominally contribute in the internal controller state data structure.
            /// This direction is used for positive "pressed" analog values, "pressed" button states, and "pressed" trigger states.
            const EPovDirection povDirectionPositive;

            /// Identifies an optional second POV direction to which this mapper should contribute in the internal controller state data structure.
            /// If present, this direction is used for negative "pressed" analog values, "not pressed" button states, and "not pressed" trigger states.
            /// If absent, no contribution is made at all for negative "pressed" analog values, "not pressed" button states, and "not pressed" trigger states.
            const std::optional<EPovDirection> maybePovDirectionNegative;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor.
            /// Specifies the single POV direction to which all updates are contributed (axis on the positive side or button/trigger pressed).
            inline constexpr PovMapper(EPovDirection povDirection) : IElementMapper(), povDirectionPositive(povDirection), maybePovDirectionNegative(std::nullopt)
            {
                // Nothing to do here.
            }

            /// Initialization constructor.
            /// Specifies two POV directions, one for positive direction updates (axis on the positive side or button/trigger pressed) and one for negative direction updates (axis on the negative side or button/trigger not pressed).
            inline constexpr PovMapper(EPovDirection povDirectionPositive, EPovDirection povDirectionNegative) : IElementMapper(), povDirectionPositive(povDirectionPositive), maybePovDirectionNegative(povDirectionNegative)
            {
                // Nothing to do here.
            }


            // -------- CONCRETE INSTANCE METHODS -------------------------- //

            void ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const override;
            void ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const override;
            void ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const override;
            SElementIdentifier GetTargetElement(void) const override;
        };
    }
}
