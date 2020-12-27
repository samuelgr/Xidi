/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Harness.h
 *   Declaration of the test harness, including program entry point.
 *****************************************************************************/

#pragma once

#include "TestCase.h"

#include <map>
#include <string>


namespace XidiTest
{
    /// Registers and runs all tests. Reports results. Implemented as a singleton object.
    /// Test cases are run in alphabetical order by name, irrespective of the order in which they are registered.
    class Harness
    {
    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Holds all registered test cases in alphabetical order.
        std::map<std::wstring, const ITestCase*> testCases;


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor. Objects cannot be constructed externally.
        Harness(void) = default;

        /// Copy constructor. Should never be invoked.
        Harness(const Harness&) = delete;


        // -------- CLASS METHODS ------------------------------------------ //

        /// Returns a reference to the singleton instance of this class.
        /// Not intended to be invoked externally.
        /// @return Reference to the singleton instance.
        static Harness& GetInstance(void);

    public:
        /// Registers a test case to be run by the harness.
        /// Typically, registration happens automatically using the #TEST_CASE macro, which is the recommended way of creating test cases.
        /// @param [in] testCase Test case object to register (appropriate instances are created automatically by the #TEST_CASE macro).
        /// @param [in] name Name of the test case (the value of the parameter passed to the #TEST_CASE macro).
        static inline void RegisterTestCase(const ITestCase* const testCase, const wchar_t* const name)
        {
            GetInstance().RegisterTestCaseInternal(testCase, name);
        }

        /// Runs all tests registered by the harness.
        /// Typically invoked only once by the entry point to the test program.
        /// @return Number of failing tests.
        static inline int RunAllTests(void)
        {
            return GetInstance().RunAllTestsInternal();
        }


    private:
        // -------- INSTANCE METHODS --------------------------------------- //

        /// Internal implementation of test case registration.
        /// @param [in] testCase Test case object to register.
        /// @param [in] name Name of the test case.
        void RegisterTestCaseInternal(const ITestCase* const testCase, const wchar_t* const name);

        /// Internal implementation of running all tests.
        /// @return Number of failing tests.
        int RunAllTestsInternal(void);
    };
}
