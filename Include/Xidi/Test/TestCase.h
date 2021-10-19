/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file TestCase.h
 *   Declaration of the test case interface.
 *****************************************************************************/

#pragma once

#include "Utilities.h"

#include <string_view>


namespace XidiTest
{
    /// Test case interface.
    class ITestCase
    {
    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor. Constructs a test case object with an associated test case name, and registers it with the harness.
        /// @param [in] name Test case name.
        ITestCase(std::wstring_view name);


        // -------- ABSTRACT INSTANCE METHODS ------------------------------ //

        /// Performs run-time checks to determine if the test case represented by this object can be run.
        /// If not, it will be skipped.
        virtual bool CanRun(void) const = 0;


        /// Runs the test case represented by this object.
        /// Implementations are generated when test cases are created using the #TEST_CASE macro.
        virtual void Run(void) const = 0;
    };

    /// Concrete test case object template.
    /// Each test case created by #TEST_CASE instantiates an object of this type with a different template parameter.
    /// @tparam kName Name of the test case.
    template <const wchar_t* kName> class TestCase : public ITestCase
    {
    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor.
        inline TestCase(void) : ITestCase(kName)
        {
            // Nothing to do here.
        }


        // -------- CONCRETE INSTANCE METHODS ------------------------------ //
        // See above for documentation.

        bool CanRun(void) const override;
        void Run(void) const override;
    };

    /// Thrown to signal a test failure. For internal use only.
    struct TestFailedException
    {
        // Empty.
    };
}

/// Exit from a test case and indicate a failing result.
#define TEST_FAILED                         throw ::XidiTest::TestFailedException()

/// Format and print a message and exit from a test case, indicating a failing result.
#define TEST_FAILED_BECAUSE(reasonf, ...)   do {::XidiTest::PrintFormatted(L"%s(%d): Test failed: " reasonf, __FILEW__, __LINE__, ##__VA_ARGS__); TEST_FAILED;} while (0)

/// Exit from a test case and indicate a failing result if the expression is false.
#define TEST_ASSERT(expr)                   do {if (!(expr)) {::XidiTest::PrintFormatted(L"%s(%d): Assertion failed: %s", __FILEW__, __LINE__, L#expr); TEST_FAILED;}} while (0)

/// Recommended way of creating test cases that execute conditionally.
/// Requires a test case name and a condition, which evaluates to a value of type bool.
/// If the condition ends up being false, which can be determined at runtime, the test case is skipped.
/// Automatically instantiates the proper test case object and registers it with the harness.
/// Treat this macro as a function declaration; the test case is the function body.
#define TEST_CASE_CONDITIONAL(name, cond) \
    inline constexpr wchar_t kTestName__##name[] = L#name; \
    ::XidiTest::TestCase<kTestName__##name>  testCaseInstance__##name; \
    bool ::XidiTest::TestCase<kTestName__##name>::CanRun(void) const { return (cond); } \
    void ::XidiTest::TestCase<kTestName__##name>::Run(void) const

/// Recommended way of creating test cases that execute unconditionally.
/// Just provide the test case name.
#define TEST_CASE(name)                     TEST_CASE_CONDITIONAL(name, true)
