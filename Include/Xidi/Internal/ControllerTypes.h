/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ControllerTypes.h
 *   Declaration of constants and types used for representing virtual controllers and their state.
 **************************************************************************************************/

#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <type_traits>

namespace Xidi
{
  namespace Controller
  {
    /// Number of physical controllers that the underlying system supports.
    /// Not all will necessarily be physically present at any given time.
    /// Maximum allowable controller identifier is one less than this value.
    inline constexpr uint16_t kPhysicalControllerCount = 4;

    /// Maximum possible reading from an XInput controller's analog stick.
    /// Value taken from XInput documentation.
    inline constexpr int32_t kAnalogValueMax = 32767;

    /// Minimum possible reading from an XInput controller's analog stick.
    /// Value derived from the above to ensure symmetry around 0.
    /// This is slightly different than the XInput API itself, which allows negative values all the
    /// way down to -32768.
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

    /// Integer type used to identify physical controllers to the underlying system interfaces.
    using TControllerIdentifier = std::remove_const_t<decltype(kPhysicalControllerCount)>;

    /// Enumerates all supported axis types using DirectInput terminology.
    /// It is not necessarily the case that all of these axes are present in a virtual controller.
    /// This enumerator just lists all the possible axes. Semantically, the value of each enumerator
    /// maps to an array position in the controller's internal state data structure.
    enum class EAxis : uint8_t
    {
      X,
      Y,
      Z,
      RotX,
      RotY,
      RotZ,

      /// Sentinel value, total number of enumerators
      Count
    };

    /// Enumerates the possible directions that can be recognized for an axis.
    /// Used for specifying the parts of an axis to which element mappers should contribute and from
    /// which force feedback actuators should obtain their input.
    enum class EAxisDirection : uint8_t
    {
      Both,
      Positive,
      Negative,

      /// Sentinel value, total number of enumerators
      Count
    };

    /// Enumerates all supported buttons.
    /// It is not necessarily the case that all of these buttons are present in a virtual
    /// controller. This enumerator just lists all the possible buttons. Semantically, the value of
    /// each enumerator maps to an array position in the controller's internal state data structure.
    enum class EButton : uint8_t
    {
      B1,
      B2,
      B3,
      B4,
      B5,
      B6,
      B7,
      B8,
      B9,
      B10,
      B11,
      B12,
      B13,
      B14,
      B15,
      B16,

      /// Sentinel value, total number of enumerators
      Count
    };

    /// Enumerates buttons that correspond to each of the possible POV directions. Xidi either
    /// presents, or does not present, a POV to the application. If a POV is presented, then these
    /// four buttons in the internal state data structure are combined into a POV reading. If not,
    /// then the corresponding part of the internal state data structure is ignored. Semantically,
    /// the value of each enumerator maps to an array position in the controller's internal state
    /// data structure.
    enum class EPovDirection : uint8_t
    {
      Up,
      Down,
      Left,
      Right,

      /// Sentinel value, total number of enumerators
      Count
    };

    /// Enumerates all types of controller elements present in the internal virtual controller
    /// state. The special whole controller value indicates that a reference is being made to the
    /// entire virtual controller rather than any specific element.
    enum class EElementType : uint8_t
    {
      Axis,
      Button,
      Pov,
      WholeController
    };

    /// Identifier for an element of a virtual controller's state. Specifies both element type and
    /// index. Valid member of the union is based on the indicated type.
    struct SElementIdentifier
    {
      EElementType type;

      union
      {
        EAxis axis;
        EButton button;
      };

      constexpr bool operator==(const SElementIdentifier& other) const
      {
        if (other.type != type) return false;

        switch (type)
        {
          case EElementType::Axis:
            return (other.axis == axis);

          case EElementType::Button:
            return (other.button == button);

          default:
            return true;
        }
      }
    };

    static_assert(sizeof(SElementIdentifier) <= 4, "Data structure size constraint violation.");

    /// Capabilities of a single Xidi virtual controller axis. Identifies the axis type enumerator
    /// and contains other information about how the axis can behave as part of a virtual
    /// controller.
    struct SAxisCapabilities
    {
      /// Type of axis.
      EAxis type : 3;

      /// Whether or not the axis supports force feedback.
      bool supportsForceFeedback : 1;

      constexpr bool operator==(const SAxisCapabilities& other) const = default;
    };

    static_assert(
        sizeof(SAxisCapabilities) == sizeof(EAxis), "Data structure size constraint violation.");
    static_assert(
        static_cast<uint8_t>(EAxis::Count) <= 0b111,
        "Highest-valued axis type identifier does not fit into 3 bits.");

    /// Capabilities of a Xidi virtual controller.
    /// Filled in by looking at a mapper and used during operations like EnumObjects to tell the
    /// application about the virtual controller's components.
    struct SCapabilities
    {
      /// Capability information for each axis present. When the controller is presented to the
      /// application, all the axes on it are presented with contiguous indices. This array is used
      /// to map from DirectInput axis index to internal axis index.
      SAxisCapabilities axisCapabilities[static_cast<int>(EAxis::Count)];

      struct
      {
        /// Number of axes in the virtual controller, also the number of elements of the axis type
        /// array that are valid.
        uint8_t numAxes : 3;

        /// Number of buttons present in the virtual controller.
        uint8_t numButtons : 5;
      };

      /// Specifies whether or not the virtual controller has a POV. If it does, then the POV
      /// buttons in the controller state are used, otherwise they are ignored.
      bool hasPov;

      constexpr bool operator==(const SCapabilities& other) const
      {
        return (
            (other.numAxes == numAxes) && (other.numButtons == numButtons) &&
            (other.hasPov == hasPov) &&
            std::equal(
                other.axisCapabilities,
                &other.axisCapabilities[numAxes],
                axisCapabilities,
                &axisCapabilities[numAxes]));
      }

      /// Appends an axis to the list of axis types in this capabilities object. Performs no
      /// bounds-checking or uniqueness-checking, so this is left to the caller to ensure.
      /// @param [in] newAxisCapabilities Axis capabilities object to append to the list of present
      /// axes.
      constexpr void AppendAxis(SAxisCapabilities newAxisCapabilities)
      {
        axisCapabilities[numAxes] = newAxisCapabilities;
        numAxes += 1;
      }

      /// Determines the index of the specified axis type within this capabilities object, if it
      /// exists.
      /// @param [in] axis Axis type for which to query.
      /// @return Index of the specified axis if it is present, or -1 if it is not.
      constexpr int FindAxis(EAxis axis) const
      {
        for (int i = 0; i < numAxes; ++i)
        {
          if (axisCapabilities[i].type == axis) return i;
        }

        return -1;
      }

      /// Computes and returns the number of axes that support force feedback.
      /// @return Number of axes mapped to force feedback actuators.
      constexpr int ForceFeedbackAxisCount(void) const
      {
        int numForceFeedbackAxes = 0;

        for (int i = 0; i < numAxes; ++i)
        {
          if (true == axisCapabilities[i].supportsForceFeedback) numForceFeedbackAxes += 1;
        }

        return numForceFeedbackAxes;
      }

      /// Determines if this capabilities object specifies a controller that has support for force
      /// feedback actuation.
      /// @return `true` if any axis is mapped to a force feedback actuator, `false` otherwise.
      constexpr bool ForceFeedbackIsSupported(void) const
      {
        for (int i = 0; i < numAxes; ++i)
        {
          if (true == axisCapabilities[i].supportsForceFeedback) return true;
        }

        return false;
      }

      /// Determines if this capabilities object specifies that the controller has an axis of the
      /// specified type and that said axis supports force feedback.
      /// @param [in] axis Axis type for which to query.
      /// @return `true` if the axis is present and supports force feedback actuation, `false`
      /// otherwise.
      constexpr bool ForceFeedbackIsSupportedForAxis(EAxis axis) const
      {
        for (int i = 0; i < numAxes; ++i)
        {
          if (axisCapabilities[i].type == axis) return axisCapabilities[i].supportsForceFeedback;
        }

        return false;
      }

      /// Checks if this capabilities object specifies that the controller has an axis of the
      /// specified type.
      /// @param [in] axis Axis type for which to query.
      /// @return `true` if the axis is present, `false` otherwise.
      constexpr bool HasAxis(EAxis axis) const
      {
        return (-1 != FindAxis(axis));
      }

      /// Checks if this capabilities object specifies that the controller has a button of the
      /// specified number.
      /// @param [in] button Button number for which to query.
      /// @return `true` if the button is present, `false` otherwise.
      constexpr bool HasButton(EButton button) const
      {
        return (static_cast<uint8_t>(button) < numButtons);
      }

      /// Checks if this capabilities object specifies that the controller has a POV.
      /// @return `true` if a POV is present, `false` otherwise.
      constexpr bool HasPov(void) const
      {
        return hasPov;
      }

      /// Checks if this capabilities object specifies that the controller has the specified
      /// element.
      /// @param [in] element Controller element for which to query.
      /// @return `true` if the element is present, `false` otherwise.
      constexpr bool HasElement(SElementIdentifier element) const
      {
        switch (element.type)
        {
          case EElementType::Axis:
            return HasAxis(element.axis);
          case EElementType::Button:
            return HasButton(element.button);
          case EElementType::Pov:
            return HasPov();
          default:
            return false;
        }
      }
    };

    static_assert(sizeof(SCapabilities) <= 8, "Data structure size constraint violation.");
    static_assert(
        static_cast<uint8_t>(EAxis::Count) <= 0b111, "Number of axes does not fit into 3 bits.");
    static_assert(
        static_cast<uint8_t>(EButton::Count) <= 0b11111,
        "Number of buttons does not fit into 5 bits.");

    /// Holds POV direction, which is presented both as an array of separate components and as a
    /// single aggregated integer view.
    union UPovDirection
    {
      /// Pressed (`true`) or unpressed (`false`) state for each POV direction separately, one
      /// element per button. Bitset versus boolean produces no size difference, given the number of
      /// POV directions.
      std::array<bool, static_cast<int>(EPovDirection::Count)> components;

      /// Aggregate state of all POV directions, available as a single quantity for easy comparison
      /// and assignment.
      uint32_t all;

      constexpr bool operator==(const UPovDirection& other) const
      {
        return (other.all == all);
      }
    };

    static_assert(
        sizeof(UPovDirection::components) == sizeof(UPovDirection::all),
        "Mismatch in POV view sizes.");

    /// Native data format for virtual controllers, used internally to represent controller state.
    /// Validity or invalidity of each element depends on the mapper.
    struct SState
    {
      /// Values for all axes, one element per axis.
      std::array<int32_t, static_cast<int>(EAxis::Count)> axis;

      /// Pressed (`true`) or unpressed (`false`) state for each button, one bit per button. Bitset
      /// is used as a size optimization, given the number of buttons.
      std::bitset<static_cast<int>(EButton::Count)> button;

      /// POV direction, presented simultaneously as individual components and as an aggregate
      /// quantity.
      UPovDirection povDirection;

      constexpr bool operator==(const SState& other) const
      {
        return (
            std::equal(
                std::cbegin(other.axis),
                std::cend(other.axis),
                std::cbegin(axis),
                std::cend(axis)) &&
            (other.button == button) && (other.povDirection == povDirection));
      }

      constexpr int32_t operator[](EAxis desiredAxis) const
      {
        return axis[static_cast<int>(desiredAxis)];
      }

      constexpr bool operator[](EButton desiredButton) const
      {
        return button[static_cast<int>(desiredButton)];
      }

      constexpr bool operator[](EPovDirection desiredPovDirection) const
      {
        return povDirection.components[static_cast<int>(desiredPovDirection)];
      }

      constexpr int32_t& operator[](EAxis desiredAxis)
      {
        return axis[static_cast<int>(desiredAxis)];
      }

      constexpr decltype(button)::reference operator[](EButton desiredButton)
      {
        return button[static_cast<int>(desiredButton)];
      }

      constexpr bool& operator[](EPovDirection desiredPovDirection)
      {
        return povDirection.components[static_cast<int>(desiredPovDirection)];
      }
    };

    static_assert(sizeof(SState) <= 32, "Data structure size constraint violation.");

    /// Enumerates possible statuses for physical controller devices.
    enum class EPhysicalDeviceStatus : uint8_t
    {
      /// Device is connected and functioning correctly
      Ok,

      /// Device is not connected and has not reported an error
      NotConnected,

      /// Device has experienced an error
      Error,

      /// Sentinel value, total number of enumerators
      Count
    };

    /// Enumerates all analog sticks that might be present on a physical controller.
    /// One enumerator exists per possible stick.
    enum class EPhysicalStick : uint8_t
    {
      LeftX,
      LeftY,
      RightX,
      RightY,

      /// Sentinel value, total number of enumerators
      Count
    };

    /// Enumerates all analog triggers that might be present on a physical controller.
    /// One enumerator exists per possible trigger.
    enum class EPhysicalTrigger : uint8_t
    {
      LT,
      RT,

      /// Sentinel value, total number of enumerators
      Count
    };

    /// Enumerates all digital buttons that might be present on a physical controller. As an
    /// implementation simplification, the order of enumerators corresponds to the ordering used in
    /// XInput. One enumerator exists per possible button. Guide and Share buttons are not actually
    /// used, but they still have space allocated for them on a speculative basis.
    enum class EPhysicalButton : uint8_t
    {
      DpadUp,
      DpadDown,
      DpadLeft,
      DpadRight,
      Start,
      Back,
      LS,
      RS,
      LB,
      RB,
      UnusedGuide,
      UnusedShare,
      A,
      B,
      X,
      Y,

      /// Sentinel value, total number of enumerators
      Count
    };

    /// Data format for representing physical controller state, as received from controller devices
    /// and before being passed through a mapper.
    struct SPhysicalState
    {
      /// Whether or not the physical state represented by this object was successfully read from a
      /// controller device.
      EPhysicalDeviceStatus deviceStatus;

      /// Analog stick values read from the physical controller, one element per possible stick and
      /// axis direction.
      std::array<int16_t, static_cast<int>(EPhysicalStick::Count)> stick;

      /// Analog trigger values read from the physical controller, one element per possible trigger.
      std::array<uint8_t, static_cast<int>(EPhysicalTrigger::Count)> trigger;

      /// Digital button values read from the physical controller, one element per possible digital
      /// button.
      std::bitset<static_cast<int>(EPhysicalButton::Count)> button;

      constexpr bool operator==(const SPhysicalState& other) const = default;

      constexpr int16_t operator[](EPhysicalStick desiredStick) const
      {
        return stick[static_cast<int>(desiredStick)];
      }

      constexpr uint8_t operator[](EPhysicalTrigger desiredTrigger) const
      {
        return trigger[static_cast<int>(desiredTrigger)];
      }

      constexpr bool operator[](EPhysicalButton desiredButton) const
      {
        return button[static_cast<int>(desiredButton)];
      }

      constexpr decltype(stick)::reference operator[](EPhysicalStick desiredStick)
      {
        return stick[static_cast<int>(desiredStick)];
      }

      constexpr decltype(trigger)::reference operator[](EPhysicalTrigger desiredTrigger)
      {
        return trigger[static_cast<int>(desiredTrigger)];
      }

      constexpr decltype(button)::reference operator[](EPhysicalButton desiredButton)
      {
        return button[static_cast<int>(desiredButton)];
      }
    };

    static_assert(sizeof(SPhysicalState) <= 16, "Data structure size constraint violation.");
  } // namespace Controller
} // namespace Xidi
