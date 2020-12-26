/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file PovMapperTest.cpp
 *   Unit tests for controller element mappers that contribute to a virtual
 *   point-of-view hat.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerElementMapper.h"
#include "ControllerTypes.h"
#include "TestCase.h"

#include <cstdint>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    // -------- TEST CASES ------------------------------------------------- //

    // Creates one button mapper for each possible virtual button and verifies that each correctly identifies its target virtual controller element.
    // Because all PovMappers contribute to the same virutal POV object (one direction per mapper), the element index is always the same.
    TEST_CASE(PovMapper_GetTargetElement)
    {
        for (int i = 0; i < (int)EPov::Count; ++i)
        {
            const PovMapper mapper((EPov)i);
            TEST_ASSERT(EElementType::Pov == mapper.GetTargetElementType());
            TEST_ASSERT(PovMapper::kPovElementIndex == mapper.GetTargetElementIndex());
        }
    }
}
