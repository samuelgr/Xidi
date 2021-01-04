/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file StateChangeEventBuffer.cpp
 *   Implementation of buffered event functionality for virtual controller
 *   state change events.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "StateChangeEventBuffer.h"

#include <atomic>
#include <boost/circular_buffer.hpp>
#include <cstdint>
#include <optional>


namespace Xidi
{
    namespace Controller
    {
        // -------- INTERNAL FUNCTIONS --------------------------------- //

        /// Computes and returns a timestamp to be used when appending an event to an event buffer.
        /// @return Timestamp value to use.
        static inline uint32_t GetTimestamp(void)
        {
            // Per DirectInput documentation, it is acceptable for the timestamp to overflow every ~50 days.
            // See IDirectInput8::GetDeviceData documentation for more information.

            return GetTickCount();
        }
        
        /// Handles a possible buffer overflow condition.
        /// @param [in] eventBuffer Event buffer object for which to check for overflow.
        /// @return `true` if the event buffer overflowed, `false` otherwise.
        static inline bool HandlePossibleOverflow(boost::circular_buffer<StateChangeEventBuffer::SEvent>& eventBuffer)
        {
            // Per DirectInput documentation, we always need one free space in the buffer.
            // This is how we ensure the number of events stored is always one less than capacity.
            // See IDirectInput8::GetDeviceData documentation for more information.
            
            const bool eventBufferWasFull = eventBuffer.full();

            if (true == eventBufferWasFull)
                eventBuffer.pop_front();

            return eventBufferWasFull;
        }

        
        // -------- INSTANCE METHODS ----------------------------------- //
        // See "StateChangeEventBuffer.h" for documentation.

        void StateChangeEventBuffer::AppendEvent(const SEventData& eventData, std::optional<uint32_t> maybeTimestamp)
        {
            // Sequence number is globally ordered with respect to all controller events, even those from other event buffers.
            static std::atomic<uint32_t> nextSequence = 0;

            const uint32_t timestamp = ((true == maybeTimestamp.has_value()) ? maybeTimestamp.value() : GetTimestamp());
            const uint32_t sequence = nextSequence++;

            eventBuffer.push_back({
                .data = eventData,
                .timestamp = timestamp,
                .sequence = sequence
            });

            eventBufferOverflowed = HandlePossibleOverflow(eventBuffer);
        }

        // --------

        void StateChangeEventBuffer::PopOldestEvents(uint32_t numEventsToPop)
        {
            for (uint32_t i = 0; (i < numEventsToPop) && (false == eventBuffer.empty()); ++i)
                eventBuffer.pop_front();

            eventBufferOverflowed = false;
        }

        // --------

        void StateChangeEventBuffer::SetCapacity(uint32_t capacity)
        {
            const uint32_t newCapacity = ((capacity > kEventBufferCapacityMax) ? kEventBufferCapacityMax : capacity);
            eventBuffer.rset_capacity(newCapacity);
            eventBufferOverflowed = HandlePossibleOverflow(eventBuffer);
        }
    }
}
