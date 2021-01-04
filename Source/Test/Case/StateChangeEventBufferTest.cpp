/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file VirtualControllerTest.cpp
 *   Unit tests for virtual controller objects.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "StateChangeEventBuffer.h"
#include "TestCase.h"

#include <cstdint>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Event data used for tests. Individual test cases can use all, or just a subset, of this test input set.
    /// Actual content does not matter (it is only checked for equality) but is nevertheless spread out over several axes and buttons.
    static constexpr StateChangeEventBuffer::SEventData kTestEventData[] = {
        {.element = {.type = EElementType::Axis,   .axis = EAxis::X},      .value = {.axis = 1122}},
        {.element = {.type = EElementType::Button, .button = EButton::B2}, .value = {.button = true}},
        {.element = {.type = EElementType::Axis,   .axis = EAxis::Y},      .value = {.axis = 3344}},
        {.element = {.type = EElementType::Pov},                           .value = {.povDirection = {false, true, false, false}}},
        {.element = {.type = EElementType::Button, .button = EButton::B7}, .value = {.button = true}},
        {.element = {.type = EElementType::Button, .button = EButton::B2}, .value = {.button = false}},
        {.element = {.type = EElementType::Pov},                           .value = {.povDirection = {false, false, false, false}}},
        {.element = {.type = EElementType::Axis,   .axis = EAxis::Z},      .value = {.axis = 5555}},
        {.element = {.type = EElementType::Pov},                           .value = {.povDirection = {false, false, false, true}}},
        {.element = {.type = EElementType::Button, .button = EButton::B1}, .value = {.button = true}},
        {.element = {.type = EElementType::Button, .button = EButton::B1}, .value = {.button = false}},
        {.element = {.type = EElementType::Axis,   .axis = EAxis::RotZ},   .value = {.axis = 6677}},
        {.element = {.type = EElementType::Axis,   .axis = EAxis::RotY},   .value = {.axis = 8888}},
        {.element = {.type = EElementType::Axis,   .axis = EAxis::RotX},   .value = {.axis = 9990}},
        {.element = {.type = EElementType::Pov},                           .value = {.povDirection = {false, false, false, false}}},
        {.element = {.type = EElementType::Button, .button = EButton::B7}, .value = {.button = false}},
    };

    /// Dummy timestamp value to use.
    /// This set of tests does not exercise timestamp generation functionality.
    constexpr uint32_t kTimestamp = 0;


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies correct behavior in the nominal case of inserting some events and then removing them in order.
    // The event buffer capacity is well above number of events being inserted, so there is no issue and therefore the buffer should never report overflow.
    // Insertion and removal is repeated enough times to ensure total volume of event data exceeds the capacity, although the buffer is always mostly empty.
    // Capacity and number of repeats are chosen to be off alignment with each other, but otherwise the specific values are not important.
    TEST_CASE(StateChangeEventBuffer_Nominal)
    {
        constexpr uint32_t kTestRepeatTimes = 17;
        constexpr uint32_t kEventBufferCapacity = ((_countof(kTestEventData) * 5) / 3);

        StateChangeEventBuffer testEventBuffer;
        testEventBuffer.SetCapacity(kEventBufferCapacity);
        TEST_ASSERT(kEventBufferCapacity == testEventBuffer.GetCapacity());
        TEST_ASSERT(0 == testEventBuffer.GetCount());

        int64_t lastSequenceSeen = INT64_MIN;
        for (int i = 0; i < kTestRepeatTimes; ++i)
        {
            // First add events, one after another, ensuring that the count increments each time.
            for (int j = 0; j < _countof(kTestEventData); ++j)
            {
                testEventBuffer.AppendEvent(kTestEventData[j], kTimestamp);
                TEST_ASSERT((j + 1) == testEventBuffer.GetCount());
                TEST_ASSERT(false == testEventBuffer.IsOverflowed());
            }

            // Next examine events without removing them.
            for (int j = 0; j < _countof(kTestEventData); ++j)
            {
                TEST_ASSERT(kTestEventData[j] == testEventBuffer[j].data);
                TEST_ASSERT(kTimestamp == testEventBuffer[j].timestamp);
                TEST_ASSERT((int64_t)testEventBuffer[j].sequence > lastSequenceSeen);
                lastSequenceSeen = (int64_t)testEventBuffer[j].sequence;
            }

            // Finally remove events one at a time, ensuring the count decrements each time and that the event removed is actually the oldest one.
            testEventBuffer.PopOldestEvents(1);
            for (int j = 1; j < _countof(kTestEventData); ++j)
            {
                TEST_ASSERT((_countof(kTestEventData) - j) == testEventBuffer.GetCount());
                TEST_ASSERT(kTestEventData[j] == testEventBuffer[0].data);
                testEventBuffer.PopOldestEvents(1);
            }
        }
    }

    // Verifies the buffer correctly retains its contents as the buffer size increases.
    // No overflow condition is triggered.
    TEST_CASE(StateChangeEventBuffer_BufferGrow)
    {
        uint32_t eventBufferCapacity = 2;
        
        StateChangeEventBuffer testEventBuffer;
        testEventBuffer.SetCapacity(eventBufferCapacity);

        // Each time an event is appended to the buffer the buffer is asked to grow by 1 event.
        for (const auto& testEventData : kTestEventData)
        {
            testEventBuffer.AppendEvent(testEventData, kTimestamp);
            TEST_ASSERT(false == testEventBuffer.IsOverflowed());

            eventBufferCapacity += 1;
            testEventBuffer.SetCapacity(eventBufferCapacity);
        }

        // Make sure the buffer successfully grew and all events are present.
        TEST_ASSERT(eventBufferCapacity == testEventBuffer.GetCapacity());
        TEST_ASSERT(_countof(kTestEventData) == testEventBuffer.GetCount());
        TEST_ASSERT(false == testEventBuffer.IsOverflowed());

        for (int i = 0; i < _countof(kTestEventData); ++i)
            TEST_ASSERT(kTestEventData[i] == testEventBuffer[i].data);
    }

    // Verifies the buffer correctly retains its contents as the buffer size increases.
    // No overflow condition is triggered. The event buffer shrinks to exactly the size needed to hold all of the events in it without overflowing.
    TEST_CASE(StateChangeEventBuffer_BufferShrink)
    {
        constexpr uint32_t kEventBufferInitialCapacity = (4 * _countof(kTestEventData));
        constexpr uint32_t kEventBufferFinalCapacity = (1 + _countof(kTestEventData));

        StateChangeEventBuffer testEventBuffer;
        testEventBuffer.SetCapacity(kEventBufferInitialCapacity);

        for (const auto& testEventData : kTestEventData)
            testEventBuffer.AppendEvent(testEventData, kTimestamp);

        testEventBuffer.SetCapacity(kEventBufferFinalCapacity);

        TEST_ASSERT(_countof(kTestEventData) == testEventBuffer.GetCount());
        TEST_ASSERT(false == testEventBuffer.IsOverflowed());

        for (int i = 0; i < _countof(kTestEventData); ++i)
            TEST_ASSERT(kTestEventData[i] == testEventBuffer[i].data);
    }

    // Verifies correct behavior in the case of an overflow due to appending more events than the buffer can hold.
    // The most recent events should remain, and the buffer should indicate an overflow condition.
    TEST_CASE(StateChangeEventBuffer_OverflowAppend)
    {
        constexpr uint32_t kEventBufferCapacity = _countof(kTestEventData) / 4;

        StateChangeEventBuffer testEventBuffer;
        testEventBuffer.SetCapacity(kEventBufferCapacity);

        for (const auto& testEventData : kTestEventData)
            testEventBuffer.AppendEvent(testEventData, kTimestamp);
        
        // Check that the number of events actually maintained in the buffer is one less than its capacity.
        // This is documented buffer behavior intended for consistency with IDirectInputDevice8::GetDeviceData.
        constexpr uint32_t kExpectedEventCount = kEventBufferCapacity - 1;
        const uint32_t kActualEventCount = testEventBuffer.GetCount();
        TEST_ASSERT(kActualEventCount == kExpectedEventCount);

        // Check the events themselves. The contents of the buffer should be the most-recently-appended events.
        for (int i = 0; i < kExpectedEventCount; ++i)
        {
            const int kEventIndex = (_countof(kTestEventData) - kExpectedEventCount) + i;
            TEST_ASSERT(testEventBuffer[i].data == kTestEventData[kEventIndex]);
        }
    }

    // Verifies correct behavior in the case of an overflow due to appending events and then shrinking the buffer.
    // The most recent events should remain, and the buffer should indicate an overflow condition.
    TEST_CASE(StateChangeEventBuffer_OverflowBufferShrink)
    {
        constexpr uint32_t kEventBufferInitialCapacity = (1 + _countof(kTestEventData));
        constexpr uint32_t kEventBufferFinalCapacity = _countof(kTestEventData) / 4;;

        StateChangeEventBuffer testEventBuffer;
        testEventBuffer.SetCapacity(kEventBufferInitialCapacity);

        for (const auto& testEventData : kTestEventData)
            testEventBuffer.AppendEvent(testEventData, kTimestamp);

        TEST_ASSERT(false == testEventBuffer.IsOverflowed());
        testEventBuffer.SetCapacity(kEventBufferFinalCapacity);
        TEST_ASSERT(true == testEventBuffer.IsOverflowed());

        constexpr uint32_t kExpectedEventCount = kEventBufferFinalCapacity - 1;
        const uint32_t kActualEventCount = testEventBuffer.GetCount();
        TEST_ASSERT(kActualEventCount == kExpectedEventCount);

        for (int i = 0; i < kExpectedEventCount; ++i)
        {
            const int kEventIndex = (_countof(kTestEventData) - kExpectedEventCount) + i;
            TEST_ASSERT(testEventBuffer[i].data == kTestEventData[kEventIndex]);
        }
    }

    // Verifies that an overflow condition is cleared by popping an event from the buffer.
    TEST_CASE(StateChangeEventBuffer_ClearOverflowOnPop)
    {
        constexpr uint32_t kEventBufferCapacity = _countof(kTestEventData) / 4;

        StateChangeEventBuffer testEventBuffer;
        testEventBuffer.SetCapacity(kEventBufferCapacity);

        for (const auto& testEventData : kTestEventData)
            testEventBuffer.AppendEvent(testEventData, kTimestamp);

        TEST_ASSERT(true == testEventBuffer.IsOverflowed());

        // Popping 0 events should be a no-op.
        testEventBuffer.PopOldestEvents(0);
        TEST_ASSERT(true == testEventBuffer.IsOverflowed());

        // Actually popping something is what is supposed to clear the overflow condition.
        testEventBuffer.PopOldestEvents(1);
        TEST_ASSERT(false == testEventBuffer.IsOverflowed());
    }

    // Verifies that an overflow condition is cleared by increasing the size of the event buffer.
    TEST_CASE(StateChangeEventBuffer_ClearOverflowOnBufferGrow)
    {
        constexpr uint32_t kEventBufferCapacity = _countof(kTestEventData) / 4;

        StateChangeEventBuffer testEventBuffer;
        testEventBuffer.SetCapacity(kEventBufferCapacity);

        for (const auto& testEventData : kTestEventData)
            testEventBuffer.AppendEvent(testEventData, kTimestamp);

        TEST_ASSERT(true == testEventBuffer.IsOverflowed());

        // Setting the same capacity as the current capacity should be a no-op.
        testEventBuffer.SetCapacity(kEventBufferCapacity);
        TEST_ASSERT(true == testEventBuffer.IsOverflowed());

        // Actually increasing the buffer size is what is supposed to clear the overflow condition.
        testEventBuffer.SetCapacity(kEventBufferCapacity + 1);
        TEST_ASSERT(false == testEventBuffer.IsOverflowed());
    }

    // Verifies that the event buffer properly empties itself without issue if the number of events to pop is in excess of the number of events present.
    TEST_CASE(StateChangeEventBuffer_PopNumberAboveCount)
    {
        constexpr uint32_t kEventBufferCapacity = _countof(kTestEventData) * 4;

        StateChangeEventBuffer testEventBuffer;
        testEventBuffer.SetCapacity(kEventBufferCapacity);

        for (const auto& testEventData : kTestEventData)
            testEventBuffer.AppendEvent(testEventData, kTimestamp);

        testEventBuffer.PopOldestEvents(kEventBufferCapacity);
        TEST_ASSERT(0 == testEventBuffer.GetCount());
    }

    // Verifies that the event buffer correctly reports is enabled and disabled status based on its capacity.
    TEST_CASE(StateChangeBuffer_EnableAndDisable)
    {
        StateChangeEventBuffer testEventBuffer;

        // By default an event buffer should be disabled.
        TEST_ASSERT(false == testEventBuffer.IsEnabled());

        // Set any capacity and it should be enabled.
        testEventBuffer.SetCapacity(1);
        TEST_ASSERT(true == testEventBuffer.IsEnabled());
        testEventBuffer.SetCapacity(2);
        TEST_ASSERT(true == testEventBuffer.IsEnabled());
        testEventBuffer.SetCapacity(3);
        TEST_ASSERT(true == testEventBuffer.IsEnabled());
        testEventBuffer.SetCapacity(4);
        TEST_ASSERT(true == testEventBuffer.IsEnabled());
        testEventBuffer.SetCapacity(50);
        TEST_ASSERT(true == testEventBuffer.IsEnabled());

        // Set the capacity to 0 again and it should be disabled.
        testEventBuffer.SetCapacity(0);
        TEST_ASSERT(false == testEventBuffer.IsEnabled());
    }
}
