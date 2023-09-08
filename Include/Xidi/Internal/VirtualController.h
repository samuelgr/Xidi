/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file VirtualController.h
 *   Declaration of a complete virtual controller.
 **************************************************************************************************/

#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>
#include <utility>

#include "ControllerTypes.h"
#include "ForceFeedbackDevice.h"
#include "ForceFeedbackTypes.h"
#include "Mapper.h"
#include "StateChangeEventBuffer.h"

namespace Xidi
{
  namespace Controller
  {
    /// Encapsulates all objects and provides all functionality needed by a complete virtual
    /// controller. Obtains state input from XInput, maps XInput data to virtual controller data,
    /// and applies transforms based on application-specified properties. Supports both
    /// instantaneous state and buffered state change events. All methods are concurrency-safe
    /// unless otherwise specified. However, bulk operations (such as reading multiple events from
    /// the event buffer) are not atomic unless the caller manually obtains a virtual controller's
    /// lock.
    class VirtualController
    {
    public:

      /// Minimum allowed value for an axis deadzone property, per DirectInput documentation.
      static constexpr uint32_t kAxisDeadzoneMin = 0;

      /// Maximum allowed value for an axis deadzone property, per DirectInput documentation.
      static constexpr uint32_t kAxisDeadzoneMax = 10000;

      /// Default value for an axis deadzone property. No deadzone region is defined by default.
      static constexpr uint32_t kAxisDeadzoneDefault = kAxisDeadzoneMin;

      /// Default lower-bound value for the range property, based on tests with real DirectInput
      /// hardware.
      static constexpr int32_t kRangeMinDefault = 0;

      /// Default upper-bound value for the range property, based on tests with real DirectInput
      /// hardware.
      static constexpr int32_t kRangeMaxDefault = 65535;

      /// Minimum allowed value for an axis saturation property, per DirectInput documentation.
      static constexpr uint32_t kAxisSaturationMin = 0;

      /// Maximum allowed value for an axis saturation property, per DirectInput documentation.
      static constexpr uint32_t kAxisSaturationMax = 10000;

      /// Default value for an axis saturation property. No saturation region is defined by default.
      static constexpr uint32_t kAxisSaturationDefault = kAxisSaturationMax;

      /// Minimum allowed value for force feedback gain, per DirectInput documentation.
      static constexpr uint32_t kFfGainMin = 0;

      /// Maximum allowed value for force feedback gain, per DirectInput documentation.
      static constexpr uint32_t kFfGainMax = 10000;

      /// Default value for force feedback gain. No scaling down of effects by default.
      static constexpr uint32_t kFfGainDefault = kFfGainMax;

      /// Permits users of the associated virtual controller to ignore certain controller elements
      /// and cause them not to generate state change events. For use with buffered events.
      class EventFilter
      {
      public:

        /// Base index for axis elements of the filter.
        static constexpr unsigned int kBaseIndexAxis = 0;

        /// Base index for button elements of the filter.
        static constexpr unsigned int kBaseIndexButton = (unsigned int)EAxis::Count;

        /// Base index for the POV element of the filter.
        static constexpr unsigned int kBaseIndexPov =
            (unsigned int)EAxis::Count + (unsigned int)EButton::Count;

        /// Computes the filter index that corresponds to a given controller element, with very
        /// little error checking.
        /// @param [in] element Controller index for which the filter index is desired.
        /// @return Corresponding filter index.
        static inline unsigned int ElementToIndex(SElementIdentifier element)
        {
          switch (element.type)
          {
            case EElementType::Axis:
              return kBaseIndexAxis + (unsigned int)element.axis;
            case EElementType::Button:
              return kBaseIndexButton + (unsigned int)element.button;
            case EElementType::Pov:
              return kBaseIndexPov;
            default:
              return (unsigned int)-1; // This should never happen.
          }
        }

        /// Adds all controller elements to the filter.
        inline EventFilter(void) : filter(ULLONG_MAX) {}

        /// Adds the specified virtual controller element to the filter so that events are generated
        /// for it.
        /// @param [in] element Desired virtual controller element.
        inline void Add(SElementIdentifier element)
        {
          filter.set(ElementToIndex(element));
        }

        /// Adds all virtual controller elements to the filter, essentially turning the filter into
        /// a no-op and generating events for all elements.
        inline void AddAll(void)
        {
          filter.set();
        }

        /// Tests if the filter contains the specified virtual controller element.
        /// @param [in] element Desired virtual controller element.
        /// @return `true` if it is contained in the filter, `false` otherwise.
        inline bool Contains(SElementIdentifier element) const
        {
          return filter.test(ElementToIndex(element));
        }

        /// Remove the specified virtual controller element from the filter so that events are not
        /// generated for it.
        /// @param [in] element Desired virtual controller element.
        inline void Remove(SElementIdentifier element)
        {
          filter.reset(ElementToIndex(element));
        }

        /// Removes all virtual controller elements from the filter, resulting in no events being
        /// generated whatsoever.
        inline void RemoveAll(void)
        {
          filter.reset();
        }

      private:

        /// Holds the filter itself, one bit per virtual controller element.
        std::bitset<(int)EAxis::Count + (int)EButton::Count + 1> filter;
      };

      /// Properties of an individual axis.
      /// Default values are roughly taken from DirectInput and XInput documentation.
      /// See DirectInput documentation for the meaning of each individual field.
      struct SAxisProperties
      {
        /// Specifies whether or not the transformations identified by this object should be enabled
        /// for the corresponding axis (in other words, should the axis properties be applied or
        /// ignored).
        bool transformationsEnabled;

        /// Deadzone of the axis, expressed as a percentage of the physical range around its center
        /// point. Can be from 0 (no deadzone) to 10000 (100% of the physical range is dead).
        uint32_t deadzone;

        /// Highest raw analog value on the positive side of the axis that falls within the deadzone
        /// region. Values at or below this should report neutral.
        int32_t deadzoneRawCutoffPositive;

        /// Lowest raw analog value on the negative side of the axis that fallw within the deadzone
        /// region. Values at or above this should report neutral.
        int32_t deadzoneRawCutoffNegative;

        /// Saturation point of the axis, expressed as a percentage of its physical range in both
        /// directions. Can be from 0 (entire axis is saturated) to 10000 (do not saturate at all).
        uint32_t saturation;

        /// Lowest raw analog value on the positive side of the axis that faills within the
        /// saturation region. Values at or above this should report extreme.
        int32_t saturationRawCutoffPositive;

        /// Minimum value in the range of raw analog values that falls within the deadzone. Values
        /// at or below this should report extreme.
        int32_t saturationRawCutoffNegative;

        /// Minimum reportable value for the axis.
        int32_t rangeMin;

        /// Maximum reportable value for the axis.
        int32_t rangeMax;

        /// Neutral value for the axis.
        int32_t rangeNeutral;

        constexpr SAxisProperties(void)
        {
          SetDeadzone(kAxisDeadzoneDefault);
          SetRange(kRangeMinDefault, kRangeMaxDefault);
          SetSaturation(kAxisSaturationDefault);
          SetTransformationsEnabled(true);
        }

        constexpr bool operator==(const SAxisProperties& other) const = default;

        /// Sets the deadzone and ensures value consistency between fields, but otherwise performs
        /// no error checking.
        /// @param [in] newDeadzone New deadzone value.
        constexpr void SetDeadzone(uint32_t newDeadzone)
        {
          deadzone = newDeadzone;
          deadzoneRawCutoffPositive = kAnalogValueNeutral +
              (((kAnalogValueMax - kAnalogValueNeutral) * (int32_t)newDeadzone) / kAxisDeadzoneMax);
          deadzoneRawCutoffNegative = kAnalogValueNeutral -
              (((kAnalogValueNeutral - kAnalogValueMin) * (int32_t)newDeadzone) / kAxisDeadzoneMax);
        }

        /// Sets the range and ensures value consistency between fields, but otherwise performs no
        /// error checking.
        /// @param [in] newRangeMin New minimum range value.
        /// @param [in] newRangeMax New maximum range value.
        constexpr void SetRange(int32_t newRangeMin, int32_t newRangeMax)
        {
          rangeMin = newRangeMin;
          rangeMax = newRangeMax;
          rangeNeutral = ((newRangeMin + newRangeMax) / 2);
        }

        /// Sets the saturation and ensures value consistency between fields, but otherwise performs
        /// no error checking.
        /// @param [in] newSaturation New saturation value.
        constexpr void SetSaturation(uint32_t newSaturation)
        {
          saturation = newSaturation;
          saturationRawCutoffPositive = kAnalogValueNeutral +
              (((kAnalogValueMax - kAnalogValueNeutral) * (int32_t)newSaturation) /
               kAxisSaturationMax);
          saturationRawCutoffNegative = kAnalogValueNeutral -
              (((kAnalogValueNeutral - kAnalogValueMin) * (int32_t)newSaturation) /
               kAxisSaturationMax);
        }

        /// Sets whether or not the transformations identified by this object should be enabled for
        /// the corresponding axis. A value of `true` indicates that axis properties should be
        /// applied and a value of `false` indicates they should be ignored.
        /// @param [in] newTransformationsEnabled Whether or not transformations should be enabled.
        constexpr void SetTransformationsEnabled(bool newTransformationsEnabled)
        {
          transformationsEnabled = newTransformationsEnabled;
        }
      };

      /// Properties that apply to the whole device.
      struct SDeviceProperties
      {
        /// Force feedback gain.
        ForceFeedback::TEffectValue ffGain;

        constexpr SDeviceProperties(void)
        {
          SetFfGain(kFfGainDefault);
        }

        constexpr bool operator==(const SDeviceProperties& other) const = default;

        /// Sets the force feedback gain. Performs no error checking.
        /// @param [in] newFfGain New force feedback gain value.
        constexpr void SetFfGain(ForceFeedback::TEffectValue newFfGain)
        {
          ffGain = newFfGain;
        }
      };

      /// Complete properties data structure.
      /// Holds all per-element and device-wide properties.
      struct SProperties
      {
        /// Axis properties, one element per possible axis.
        std::array<SAxisProperties, static_cast<int>(EAxis::Count)> axis;

        /// Device-wide properties.
        SDeviceProperties device;

        constexpr bool operator==(const SProperties& other) const = default;

        /// Provides read-only access to axis properties by indexing using an enumerator.
        /// @param [in] desiredAxis Enumerator that identifies the desired axis.
        /// @return Associated axis property data.
        constexpr decltype(axis)::const_reference operator[](EAxis desiredAxis) const
        {
          return axis[static_cast<int>(desiredAxis)];
        }

        /// Provides mutable access to axis properties by indexing using an enumerator.
        /// @param [in] desiredAxis Enumerator that identifies the desired axis.
        /// @return Associated axis property data.
        constexpr decltype(axis)::reference operator[](EAxis desiredAxis)
        {
          return axis[static_cast<int>(desiredAxis)];
        }
      };

      VirtualController(TControllerIdentifier controllerId);

      VirtualController(const VirtualController& other) = delete;

      /// Cleans up and terminates the background monitoring thread, and unregisters this controller
      /// for force feedback.
      ~VirtualController(void);

      /// Modifies the contents of the specified controller state object by applying this virtual
      /// controller's properties. Primarily intended for internal use but exposed for testing
      /// purposes. Implementation is not concurrency-safe.
      /// @param [in,out] controllerState Controller state object to transform.
      void ApplyProperties(SState& controllerState) const;

      /// Adds the specified virtual controller element to this virtual controller's event filter so
      /// that events are generated for it.
      /// @param [in] element Desired virtual controller element.
      inline void EventFilterAddElement(SElementIdentifier element)
      {
        eventFilter.Add(element);
      }

      /// Adds all virtual controller elements to this virtual controller's event filter filter,
      /// essentially turning the filter into a no-op and generating events for all elements.
      inline void EventFilterAddAllElements(void)
      {
        eventFilter.AddAll();
      }

      /// Removes the specified virtual controller element from this virtual controller's event
      /// filter so that events are not generated for it.
      /// @param [in] element Desired virtual controller element.
      inline void EventFilterRemoveElement(SElementIdentifier element)
      {
        eventFilter.Remove(element);
      }

      /// Removes all virtual controller elements from this virtual controller's event filter,
      /// resulting in no events being generated whatsoever.
      inline void EventFilterRemoveAllElements(void)
      {
        eventFilter.RemoveAll();
      }

      /// Allows access to the force feedback device buffer on the physical controller associated
      /// with this virtual controller.
      /// @return Pointer to the buffer if this controller is registered with the physical
      /// controller, `nullptr` otherwise.
      inline ForceFeedback::Device* ForceFeedbackGetDevice(void) const
      {
        return physicalControllerForceFeedbackBuffer;
      }

      /// Determines if this object is registered for force feedback operations with its associated
      /// physical controller.
      /// @return `true` if so, `false` if not.
      inline bool ForceFeedbackIsRegistered(void) const
      {
        return (nullptr != physicalControllerForceFeedbackBuffer);
      }

      /// Attempts to registers this object for force feedback operations with its associated
      /// physical controller. Only one virtual controller object can ever be registered for force
      /// feedback operations at any given time. This is conceptually equivalent to acquiring a
      /// device in "exclusive" mode. Registration is co-operative, meaning that this operation will
      /// fail if another object is already registered.
      /// @return `true` if this object was already registered or is now successfully registered,
      /// `false` otherwise.
      bool ForceFeedbackRegister(void);

      /// Unregisters this object for force feedback operations with its associated physical
      /// controller.
      void ForceFeedbackUnregister(void);

      /// Retrieves and returns the capabilities of this virtual controller.
      /// Controller capabilities act as metadata that are used internally and can be presented to
      /// applications.
      /// @return Data structure representing the capabilities of this virtual controller.
      SCapabilities GetCapabilities(void) const;

      /// Retrieves and returns the deadzone property of the specified axis.
      /// @param [in] axis Target axis.
      /// @return Deadzone value associated with the target axis.
      inline uint32_t GetAxisDeadzone(EAxis axis) const
      {
        return properties[axis].deadzone;
      }

      /// Retrieves and returns the range property of the specified axis.
      /// @param [in] axis Target axis.
      /// @return Pair of range values associated with the target axis. First is the minimum, and
      /// second is the maximum.
      inline std::pair<int32_t, int32_t> GetAxisRange(EAxis axis) const
      {
        return std::make_pair(properties[axis].rangeMin, properties[axis].rangeMax);
      }

      /// Retrieves and returns the saturation property of the specified axis.
      /// @param [in] axis Target axis.
      /// @return Saturation value associated with the target axis.
      inline uint32_t GetAxisSaturation(EAxis axis) const
      {
        return properties[axis].saturation;
      }

      /// Retrieves and returns whether or not values read from the physical controller for the
      /// specified axis will be transformed by properties such as deadzone, saturation, and range.
      /// @param [in] axis Target axis.
      /// @return Whether or not transformationso are enabled for the target axis.
      inline bool GetAxisTransformationsEnabled(EAxis axis) const
      {
        return properties[axis].transformationsEnabled;
      }

      /// Retrieves and returns the capacity of the event buffer in number of events.
      /// @return Capacity of the event buffer.
      inline uint32_t GetEventBufferCapacity(void) const
      {
        return eventBuffer.GetCapacity();
      }

      /// Retrieves and returns the number of events held in the event buffer.
      /// @return Event count of the event buffer.
      inline uint32_t GetEventBufferCount(void) const
      {
        return eventBuffer.GetCount();
      }

      /// Retrieves a read-only reference to a buffered event at the specified index, without
      /// performing any bounds-checking. Event with index 0 is the oldest, and higher indices
      /// indicate more recent events. To prevent the event buffer from being modified while
      /// accessing multiple events, the caller should first obtain this virtual controller's lock.
      /// @param [in] index Index of the desired event.
      /// @return Read-only reference to the event at the desired index.
      inline const StateChangeEventBuffer::SEvent& GetEventBufferEvent(uint32_t index) const
      {
        return eventBuffer[index];
      }

      /// Retrieves and returns the force feedback gain property for this controller.
      /// @return Force feedback gain property value.
      inline uint32_t GetForceFeedbackGain(void) const
      {
        return (uint32_t)properties.device.ffGain;
      }

      /// Retrieves and returns this controller's identifier.
      /// @return This controller's identifier.
      inline TControllerIdentifier GetIdentifier(void) const
      {
        return kControllerIdentifier;
      }

      /// Retrieves and returns the latest view of the state of this virtual controller.
      /// @return Current state of this virtual controller.
      SState GetState(void);

      /// Checks if this virtual controller has a state change event handle which would be signalled
      /// on virtual controller state change.
      /// @return `true` if so, `false` otherwise.
      inline bool HasStateChangeEventHandle(void) const
      {
        return (NULL != stateChangeEventHandle);
      }

      /// Checks if the event buffering is enabled.
      /// @return `true` if event buffering is enabled, `false` otherwise.
      inline bool IsEventBufferEnabled(void) const
      {
        return eventBuffer.IsEnabled();
      }

      /// Checks if an overflow condition has occurred on this virtual controller's event buffer.
      /// @return `true` if an overflow condition is present, `false` otherwise.
      inline bool IsEventBufferOverflowed(void) const
      {
        return eventBuffer.IsOverflowed();
      }

      /// Locks this virtual controller for ensuring proper concurrency control.
      /// The returned lock object is scoped and, as a result, will automatically unlock this
      /// virtual controller upon its destruction. Used internally for this purpose, and can be used
      /// externally for locking ahead of bulk events or direct event buffer access.
      /// @return Scoped lock object that has acquired this virtual controller's concurrency control
      /// mutex.
      inline std::unique_lock<std::recursive_mutex> Lock(void)
      {
        return std::unique_lock(controllerMutex);
      }

      /// Removes and discards up to the specified number of the oldest events from this virtual
      /// controller's event buffer and clears any present overflow condition.
      /// @param [in] numEventsToPop Maximum number of events to remove.
      void PopEventBufferOldestEvents(uint32_t numEventsToPop);

      /// Generates this virtual controller's processed state view by applying this virtual
      /// controller's properties to its raw state view. Not concurrency-safe, and primarily
      /// intended for internal use.
      void ReapplyProperties(void);

      /// Refreshes the virtual controller's state using the supplied new state data.
      /// Primarily intended to be called by a background thread, but exposed externally for
      /// testing.
      /// @param [in] newRawVirtualStateData Raw virtual controller state data to apply to this
      /// virtual controller's internal state view.
      /// @return `true` if the state of the controller changed as a result of applying the new
      /// state data, `false` otherwise.
      bool RefreshState(SState newRawVirtualStateData);

      /// Sets the deadzone property for a single axis.
      /// @param [in] axis Target axis.
      /// @param [in] deadzone Desired deadzone value.
      /// @return `true` if the new deadzone value was successfully validated and set, `false`
      /// otherwise.
      bool SetAxisDeadzone(EAxis axis, uint32_t deadzone);

      /// Sets the range property for a single axis.
      /// @param [in] axis Target axis.
      /// @param [in] rangeMin Desired minimum range value.
      /// @param [in] rangeMax Desired maximum range value.
      /// @return `true` if the new range was successfully validated and set, `false` otherwise.
      bool SetAxisRange(EAxis axis, int32_t rangeMin, int32_t rangeMax);

      /// Sets the saturation property for a single axis.
      /// @param [in] axis Target axis.
      /// @param [in] saturation Desired saturation value.
      /// @return `true` if the new saturation value was successfully validated and set, `false`
      /// otherwise.
      bool SetAxisSaturation(EAxis axis, uint32_t saturation);

      /// Enables or disables transformation of raw values read from the physical device using
      /// properties like deadzone, saturation, and range for the specified axis.
      /// @param [in] axis Target axis.
      /// @param [in] transformationsEnabled Whether or not transformations should be enabled for
      /// the target axis.
      void SetAxisTransformationsEnabled(EAxis axis, bool transformationsEnabled);

      /// Sets the deadzone property for all axes.
      /// @param [in] deadzone Desired deadzone value.
      /// @return `true` if the new deadzone value was successfully validated and set, `false`
      /// otherwise.
      bool SetAllAxisDeadzone(uint32_t deadzone);

      /// Sets the range property for all axes.
      /// @param [in] rangeMin Desired minimum range value.
      /// @param [in] rangeMax Desired maximum range value.
      /// @return `true` if the new range was successfully validated and set, `false` otherwise.
      bool SetAllAxisRange(int32_t rangeMin, int32_t rangeMax);

      /// Sets the saturation property for all axes.
      /// @param [in] saturation Desired saturation value.
      /// @return `true` if the new saturation value was successfully validated and set, `false`
      /// otherwise.
      bool SetAllAxisSaturation(uint32_t saturation);

      /// Enables or disables transformation of raw values read from the physical device using
      /// properties like deadzone, saturation, and range for all axes.
      /// @param [in] transformationsEnabled Whether or not transformations should be enabled for
      /// the target axis.
      void SetAllAxisTransformationsEnabled(bool transformationsEnabled);

      /// Sets the event buffer capacity.
      /// @param [in] capacity Desired event buffer capacity in number of events.
      /// @return `true` if the new event buffer capacity was successfully validated and set,
      /// `false` otherwise.
      bool SetEventBufferCapacity(uint32_t capacity);

      /// Sets the force feedback gain property for this controller.
      /// @param [in] ffGain Desired force feedback gain value.
      /// @return `true` if the new force feedback gain value was successfully validated and set,
      /// `false` otherwise.
      bool SetForceFeedbackGain(uint32_t ffGain);

      /// Sets the state change event handle, which will be signalled whenever a virtual controller
      /// state change occurs.
      /// @param [in] eventHandle New event handle to set, or `NULL` to disable notifications
      /// entirely.
      void SetStateChangeEvent(HANDLE eventHandle);

      /// Signals the state change event.
      /// Intended to be invoked internally.
      void SignalStateChangeEvent(void);

    private:

      /// Controller identifier to be used when communicating with the underlying real controller.
      const TControllerIdentifier kControllerIdentifier;

      /// Provides concurrency control to the data structures in this virtual controller.
      std::recursive_mutex controllerMutex;

      /// Buffer for holding controller state change events.
      StateChangeEventBuffer eventBuffer;

      /// Filter to be used for deciding which controller elements are allowed to generate buffered
      /// events. Default state is all controller elements are included in the filter.
      EventFilter eventFilter;

      /// All properties associated with this virtual controller.
      SProperties properties;

      /// State of the virtual controller as of the last refresh.
      /// Raw values, with no properties or other processing applied.
      SState stateRaw;

      /// State of the virtual controller as of the last refresh.
      /// Fully processed, all properties have been applied.
      SState stateProcessed;

      /// State change event notification handle, optionally provided by applications.
      /// The underlying event object is owned by the application, not by this object.
      HANDLE stateChangeEventHandle;

      /// Background thread that monitors the associated physical controller for updates.
      std::thread physicalControllerMonitor;

      /// Used to indicate that the physical controller monitor thread should stop running.
      std::stop_source physicalControllerMonitorStop;

      /// Pointer to the physical device force feedback buffer. Valid only if this virtual
      /// controller object is registered for force feedback, `nullptr` all other times.
      ForceFeedback::Device* physicalControllerForceFeedbackBuffer;
    };
  } // namespace Controller
} // namespace Xidi
