/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ElementMapper.h
 *   Declaration of functionality used to implement mappings from individual XInput controller
 *   elements to virtual DirectInput controller elements.
 **************************************************************************************************/

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "ControllerTypes.h"
#include "Keyboard.h"
#include "Mouse.h"

namespace Xidi
{
  namespace Controller
  {
    /// Interface for mapping an XInput controller element's state reading to an internal controller
    /// state data structure value. An instance of this object exists for each XInput controller
    /// element in a mapper.
    class IElementMapper
    {
    public:

      virtual ~IElementMapper(void) = default;

      /// Allocates, constructs, and returns a pointer to a copy of this element mapper.
      /// @return Smart pointer to a copy of this element mapper.
      virtual std::unique_ptr<IElementMapper> Clone(void) const = 0;

      /// Calculates the contribution to controller state from a given analog reading in the
      /// standard XInput axis range -32768 to +32767. Contribution is aggregated with anything that
      /// already exists in the controller state.
      /// @param [in,out] controllerState Controller state data structure to be updated.
      /// @param [in] analogStickValue Raw analog stick value from the XInput controller.
      /// @param [in] sourceIdentifier Opaque identifier for the specific controller element that is
      /// triggering the contribution.
      virtual void ContributeFromAnalogValue(
          SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const = 0;

      /// Calculates the contribution to controller state from a given button pressed status
      /// reading. Contribution is aggregated with anything that already exists in the controller
      /// state.
      /// @param [in,out] controllerState Controller state data structure to be updated.
      /// @param [in] buttonPressed Button state from the XInput controller, `true` if pressed and
      /// `false` otherwise.
      /// @param [in] sourceIdentifier Opaque identifier for the specific controller element that is
      /// triggering the contribution.
      virtual void ContributeFromButtonValue(
          SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const = 0;

      /// Calculates the contribution to controller state from a given trigger reading in the
      /// standard XInput trigger range 0 to 255. Contribution is aggregated with anything that
      /// already exists in the controller state.
      /// @param [in,out] controllerState Controller state data structure to be updated.
      /// @param [in] buttonPressed Button state from the XInput controller, `true` if pressed and
      /// `false` otherwise.
      /// @param [in] sourceIdentifier Opaque identifier for the specific controller element that is
      /// triggering the contribution.
      virtual void ContributeFromTriggerValue(
          SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const = 0;

      /// Specifies that the element mapper should make a neutral state contribution to the virtual
      /// controller. Primarily intended for element mappers that have side effects so that they can
      /// reset their side effects in response to not making any contribution. It is optional to
      /// override this method, as a default empty implementation is supplied.
      /// @param [in,out] controllerState Controller state data structure to be updated.
      /// @param [in] sourceIdentifier Opaque identifier for the specific controller element that is
      /// triggering the contribution.
      virtual void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const {}

      /// Specifies the number of virtual controller elements that are the target of any
      /// contributions from this element mapper.
      /// @return Number of virtual controller elements to which contributions are targetted.
      virtual int GetTargetElementCount(void) const = 0;

      /// Specifies one of the virtual controller elements that is the target of any contributions
      /// from this element mapper.
      /// @param [in] index Index of the target element. Must be less than the count reported by
      /// #GetTargetElementCount.
      /// @return Identifier of the targert virtual controller element, if it exists.
      virtual std::optional<SElementIdentifier> GetTargetElementAt(int index) const = 0;
    };

    /// Maps a single XInput controller element such that it contributes to an axis value on a
    /// virtual controller. For analog sticks and triggers, the value read is mapped directly to the
    /// corresponding virtual controller axis. Half-axis mode generally makes sense only for
    /// triggers because they can share an axis, but it is implemented by range mapping for analog
    /// stick axes as well. For buttons, the value is either negative extreme if the button is not
    /// pressed or positive extreme if the value is pressed. Use a half-axis configuration to map to
    /// either neutral (not pressed) or extreme value (pressed).
    class AxisMapper : public IElementMapper
    {
    public:

      inline constexpr AxisMapper(EAxis axis, EAxisDirection direction = EAxisDirection::Both)
          : IElementMapper(), axis(axis), direction(direction)
      {}

      /// Retrieves and returns the axis to which this mapper should contribute.
      /// @return Target axis.
      inline EAxis GetAxis(void) const
      {
        return axis;
      }

      /// Retrieves and returns the axis direction to which this mapper should contribute on its
      /// associated axis.
      /// @return Target axis direction.
      inline EAxisDirection GetAxisDirection(void) const
      {
        return direction;
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Identifies the axis to which this mapper should contribute in the internal
      /// controller state data structure.
      const EAxis axis;

      /// Identifies the direction to which this mapper should contribute on its associated
      /// axis. If set to anything other than both directions, the contribution is to half of
      /// the axis only.
      const EAxisDirection direction;
    };

    /// Maps a single XInput controller element such that it contributes to a button reading on a
    /// virtual controller. For analog sticks, if the axis displacement from neutral is greater than
    /// a threshold, the button is considered pressed. For triggers, if the magnitude of the trigger
    /// reading is greater than a threshold, the button is considered pressed. For buttons, the
    /// button state is mapped directly to the target button.
    class ButtonMapper : public IElementMapper
    {
    public:

      inline constexpr ButtonMapper(EButton button) : IElementMapper(), button(button) {}

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Identifies the button to which this mapper should contribute in the internal
      /// controller state data structure.
      const EButton button;
    };

    /// Maps a single XInput controller element to multiple underlying element mappers.
    class CompoundMapper : public IElementMapper
    {
    public:

      /// Maximum number of underlying element mappers that can be present.
      static constexpr int kMaxUnderlyingElementMappers = 8;

      /// Convenience alias for the type used to hold underlying element mappers.
      using TElementMappers =
          std::array<std::unique_ptr<IElementMapper>, kMaxUnderlyingElementMappers>;

      inline CompoundMapper(TElementMappers&& elementMappers)
          : elementMappers(std::move(elementMappers))
      {}

      inline CompoundMapper(const CompoundMapper& other)
          : elementMappers(CopyElementMappers(other.GetElementMappers()))
      {}

      /// Retrieves and returns a read-only reference to the underlying element mapper array.
      /// @return Read-only reference to the underlying element mapper array.
      inline const TElementMappers& GetElementMappers(void) const
      {
        return elementMappers;
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Copies the underlying element mapper data structure. For internal use only.
      /// @param [in] elementMappers Source element mapper data structure.
      /// @return Copy of the source element mapper data structure.
      static inline TElementMappers CopyElementMappers(const TElementMappers& elementMappers)
      {
        TElementMappers copyOfElementMappers;

        for (size_t i = 0; i < elementMappers.size(); ++i)
        {
          if (nullptr == elementMappers[i])
            copyOfElementMappers[i] = nullptr;
          else
            copyOfElementMappers[i] = elementMappers[i]->Clone();
        }

        return copyOfElementMappers;
      }

      /// Element mappers to which input is forwarded.
      const TElementMappers elementMappers;
    };

    /// Maps a single XInput controller element such that it contributes to an axis value on a
    /// virtual controller, but removes analog functionality. Values contributed are either zero or
    /// extreme. For analog sticks, the value read is mapped to either neutral or an extreme axis
    /// value. In whole-axis mode, the possible values are negative extreme, neutral, and positive
    /// extreme. In half-axis mode, possible values are neutral and extreme (input in the inactive
    /// direction is ignored). For triggers, which unlike analog sticks do not have a centered
    /// neutral position, possible values depend on the axis mode. In whole-axis mode, the possible
    /// values are negative extreme and positive extreme. In half-axis mode, the possible values are
    /// neutral and extreme. For buttons, the behavior is the same as the standard button-to-axis
    /// mapping behavior.
    class DigitalAxisMapper : public AxisMapper
    {
    public:

      inline constexpr DigitalAxisMapper(
          EAxis axis, EAxisDirection direction = EAxisDirection::Both)
          : AxisMapper(axis, direction)
      {}

      // AxisMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
    };

    /// Inverts the input reading from an XInput controller element and then forwards it to another
    /// element mapper.
    class InvertMapper : public IElementMapper
    {
    public:

      inline InvertMapper(std::unique_ptr<const IElementMapper>&& elementMapper)
          : elementMapper(std::move(elementMapper))
      {}

      inline InvertMapper(const InvertMapper& other)
          : elementMapper((other.elementMapper != nullptr) ? other.elementMapper->Clone() : nullptr)
      {}

      /// Retrieves and returns a raw read-only pointer to the underlying element mapper. This
      /// object maintains ownership over the returned pointer.
      /// @return Read-only pointer to the underlying element mapper.
      inline const IElementMapper* GetElementMapper(void) const
      {
        return elementMapper.get();
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Mapper to which inverted input is forwarded.
      const std::unique_ptr<const IElementMapper> elementMapper;
    };

    /// Maps a single XInput controller element to a keyboard key.
    /// For analog sticks, if the axis displacement from neutral is greater than a threshold, the
    /// keyboard key is considered pressed. For triggers, if the magnitude of the trigger reading is
    /// greater than a threshold, the keyboard key is considered pressed. For buttons, the button
    /// state is mapped directly to the target keyboard key.
    class KeyboardMapper : public IElementMapper
    {
    public:

      inline constexpr KeyboardMapper(Keyboard::TKeyIdentifier key) : key(key) {}

      /// Retrieves and returns the target keyboard key to which this object contributes.
      /// Intended for tests.
      /// @return Target keyboard key identifier.
      inline Keyboard::TKeyIdentifier GetTargetKey(void) const
      {
        return key;
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Identifies the keyboard key to which this mapper should contribute on the virtual
      /// keyboard.
      const Keyboard::TKeyIdentifier key;
    };

    /// Maps a single XInput controller element to a mouse movement axis.
    /// For analog sticks and triggers, the value read is mapped directly to the corresponding mouse
    /// axis, scaled by an appropriate speed. Half-axis mode generally makes sense only for triggers
    /// because they can share an axis, but it is implemented by range mapping for analog stick axes
    /// as well. For buttons, the value is either negative extreme if the button is not pressed or
    /// positive extreme if the value is pressed. Use a half-axis configuration to map to either
    /// neutral (not pressed) or extreme value (pressed).
    class MouseAxisMapper : public IElementMapper
    {
    public:

      inline constexpr MouseAxisMapper(
          Mouse::EMouseAxis axis, EAxisDirection direction = EAxisDirection::Both)
          : IElementMapper(), axis(axis), direction(direction)
      {}

      /// Retrieves and returns the mouse axis with which this element mapper is associated.
      /// @return Associated axis.
      inline Mouse::EMouseAxis GetAxis(void) const
      {
        return axis;
      }

      /// Retrieves and returns the axis direction to which this element mapper should contribute on
      /// its associated axis.
      /// @return Target axis direction.
      inline EAxisDirection GetAxisDirection(void) const
      {
        return direction;
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const override;
      void ContributeFromButtonValue(
          SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const override;
      void ContributeFromTriggerValue(
          SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const override;
      void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Identifies the axis to which this mapper should contribute in the internal controller
      /// state data structure.
      const Mouse::EMouseAxis axis;

      /// Identifies the direction to which this mapper should contribute on its associated axis.
      /// If set to anything other than both directions, the contribution is to half of the axis
      /// only.
      const EAxisDirection direction;
    };

    /// Maps a single XInput controller element to a mouse button.
    /// For analog sticks, if the axis displacement from neutral is greater than a threshold, the
    /// mouse button is considered pressed. For triggers, if the magnitude of the trigger reading is
    /// greater than a threshold, the mouse button is considered pressed. For buttons, the button
    /// state is mapped directly to the target mouse button.
    class MouseButtonMapper : public IElementMapper
    {
    public:

      inline constexpr MouseButtonMapper(Mouse::EMouseButton mouseButton) : mouseButton(mouseButton)
      {}

      /// Retrieves and returns the target mouse button to which this object contributes.
      /// @return Target mouse button identifier.
      inline Mouse::EMouseButton GetMouseButton(void) const
      {
        return mouseButton;
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Identifies the mouse button to which this mapper should contribute on the virtual mouse.
      const Mouse::EMouseButton mouseButton;
    };

    /// Maps a single XInput controller element to a modifier that changes the mouse speed scaling
    /// factor for other element mappers that affect the sysetm mouse.
    class MouseSpeedModifierMapper : public IElementMapper
    {
    public:

      inline constexpr MouseSpeedModifierMapper(unsigned int mouseSpeedScalingFactor)
          : mouseSpeedScalingFactor(mouseSpeedScalingFactor)
      {}

      /// Retrieves and returns the target mouse button to which this object contributes.
      /// @return Target mouse button identifier.
      inline unsigned int GetMouseSpeedScalingFactor(void) const
      {
        return mouseSpeedScalingFactor;
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Holds the mouse speed scaling factor that this element mapper will use as an override when
      /// this modifier is active.
      const unsigned int mouseSpeedScalingFactor;
    };

    /// Maps a single XInput controller element such that it contributes to a POV on a virtual
    /// controller.
    class PovMapper : public IElementMapper
    {
    public:

      inline constexpr PovMapper(EPovDirection povDirection)
          : IElementMapper(), povDirection(povDirection)
      {}

      /// Retrieves and returns the direction used for contributions.
      /// @return Target direction for positive contributions.
      inline EPovDirection GetDirection(void) const
      {
        return povDirection;
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Identifies the POV direction to which this mapper should contribute in the internal
      /// controller state data structure.
      const EPovDirection povDirection;
    };

    /// Maps a single XInput controller element to two underlying element mappers depending on its
    /// state, either positive or negative. For analog values, "positive" means that the axis value
    /// is greater than or equal to the netural value, and "negative" means it is less than the
    /// neutral value. For button values, "positive" means the button is pressed, and "negative"
    /// means it is not pressed. For trigger values, "positive" means the trigger value is greater
    /// than or equal to the midpoint, and "negative" means it is less than the midpoint. Whichever
    /// of the two contained element mapper is inactive during any given request for contributions
    /// is given an opportunity to contribute a neutral state.
    class SplitMapper : public IElementMapper
    {
    public:

      inline SplitMapper(
          std::unique_ptr<const IElementMapper>&& positiveMapper,
          std::unique_ptr<const IElementMapper>&& negativeMapper)
          : positiveMapper(std::move(positiveMapper)), negativeMapper(std::move(negativeMapper))
      {}

      inline SplitMapper(const SplitMapper& other)
          : positiveMapper(
                (other.positiveMapper != nullptr) ? other.positiveMapper->Clone() : nullptr),
            negativeMapper(
                (other.negativeMapper != nullptr) ? other.negativeMapper->Clone() : nullptr)
      {}

      /// Retrieves and returns a raw read-only pointer to the positive element mapper. This object
      /// maintains ownership over the returned pointer.
      /// @return Read-only pointer to the positive element mapper.
      inline const IElementMapper* GetPositiveMapper(void) const
      {
        return positiveMapper.get();
      }

      /// Retrieves and returns a raw read-only pointer to the negative element mapper. This object
      /// maintains ownership over the returned pointer.
      /// @return Read-only pointer to the negative element mapper.
      inline const IElementMapper* GetNegativeMapper(void) const
      {
        return negativeMapper.get();
      }

      // IElementMapper
      std::unique_ptr<IElementMapper> Clone(void) const override;
      void ContributeFromAnalogValue(
          SState& controllerState,
          int16_t analogValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromButtonValue(
          SState& controllerState,
          bool buttonPressed,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeFromTriggerValue(
          SState& controllerState,
          uint8_t triggerValue,
          uint32_t sourceIdentifier = 0) const override;
      void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier = 0) const override;
      int GetTargetElementCount(void) const override;
      std::optional<SElementIdentifier> GetTargetElementAt(int index) const override;

    private:

      /// Underlying mapper that is asked for a contribution when the associated XInput
      /// controller element is in "positive" state.
      const std::unique_ptr<const IElementMapper> positiveMapper;

      /// Underlying mapper that is asked for a contribution when the associated XInput
      /// controller element is in "negative" state.
      const std::unique_ptr<const IElementMapper> negativeMapper;
    };
  } // namespace Controller
} // namespace Xidi
