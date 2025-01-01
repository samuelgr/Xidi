/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file StateChangeEventBuffer.cpp
 *   Implementation of buffered event functionality for virtual controller state change events.
 **************************************************************************************************/

#include "StateChangeEventBuffer.h"

#include <atomic>
#include <cstdint>

#include <boost/circular_buffer.hpp>

#include "ApiWindows.h"
#include "ControllerTypes.h"

namespace Xidi
{
  namespace Controller
  {
    /// Handles a possible buffer overflow condition.
    /// @param [in] eventBuffer Event buffer object for which to check for overflow.
    /// @return `true` if the event buffer overflowed, `false` otherwise.
    static inline bool HandlePossibleOverflow(
        boost::circular_buffer<StateChangeEventBuffer::SEvent>& eventBuffer)
    {
      // Per DirectInput documentation, we always need one free space in the buffer.
      // This is how we ensure the number of events stored is always one less than capacity.

      const bool eventBufferWasFull = ((0 != eventBuffer.size()) && (true == eventBuffer.full()));

      if (true == eventBufferWasFull) eventBuffer.pop_front();

      return eventBufferWasFull;
    }

    void StateChangeEventBuffer::AppendEvent(SEventData eventData, uint32_t timestamp)
    {
      // Sequence number is globally ordered with respect to all controller events, even those from
      // other event buffers.
      static std::atomic<uint32_t> nextSequence = 0;

      eventBuffer.push_back(
          {.data = eventData, .timestamp = timestamp, .sequence = nextSequence++});

      eventBufferOverflowed = HandlePossibleOverflow(eventBuffer);
    }

    void StateChangeEventBuffer::PopOldestEvents(uint32_t numEventsToPop)
    {
      // Popping 0 events is a no-op.
      if (numEventsToPop > 0)
      {
        for (uint32_t i = 0; (i < numEventsToPop) && (false == eventBuffer.empty()); ++i)
          eventBuffer.pop_front();

        eventBufferOverflowed = false;
      }
    }

    void StateChangeEventBuffer::SetCapacity(uint32_t capacity)
    {
      // Setting the capacity to the same as the current capacity is a no-op.
      if (GetCapacity() != capacity)
      {
        const uint32_t newCapacity =
            ((capacity > kEventBufferCapacityMax) ? kEventBufferCapacityMax : capacity);
        eventBuffer.rset_capacity(newCapacity);
        eventBufferOverflowed = HandlePossibleOverflow(eventBuffer);
      }
    }
  } // namespace Controller
} // namespace Xidi
