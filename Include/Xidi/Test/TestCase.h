/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file TestCase.h
 *   Declaration of the test case interface.
 *****************************************************************************/

#pragma once


namespace XidiTest
{
    /// Test case interface.
    class ITestCase
    {
    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor. Constructs a test case object with an associated test case name, and registers it with the harness.
        /// @param [in] name Test case name.
        ITestCase(const wchar_t* const name);


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Prints the specified message and appends a newline.
        /// Available to be called directly from the body of a test case.
        /// @param [in] str Message string.
        void Print(const wchar_t* const str) const;

        /// Formats and prints the specified message and appends a newline.
        /// Available to be called directly from the body of a test case.
        /// @param [in] format Message string, possibly with format specifiers.
        void PrintFormatted(const wchar_t* const format, ...) const;


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

    struct TestFailedException
    {
        const wchar_t* const reason;

        TestFailedException(const wchar_t* reason = nullptr) : reason(reason)
        {
            // Nothing to do here.
        }
    };
}

/// Exit from a test case and indicate a failing result without a specific reason.
#define TEST_FAILED                         throw TestFailedException()

/// Exit from a test case and indicate a failing result with the given reason as a null-terminated string.
#define TEST_FAILED_BECAUSE(cwstrReason)    throw TestFailedException(cwstrReason)

/// Exit from a test case and indicate a failing result if the expression is false.
#define TEST_ASSERT(expr)                   do {if (!(expr)) {PrintFormatted(L"%s:%d: Assertion failed: %s", __FILEW__, __LINE__, L#expr); TEST_FAILED;}} while (0)

/// Recommended way of creating test cases that execute conditionally.
/// Requires a test case name and a condition, which evaluates to a value of type bool.
/// If the condition ends up being false, which can be determined at runtime, the test case is skipped.
/// Automatically instantiates the proper test case object and registers it with the harness.
/// Treat this macro as a function declaration; the test case is the function body.
#define TEST_CASE_CONDITIONAL(name, cond) \
    inline constexpr wchar_t kTestName__##name[] = L#name; \
    XidiTest::TestCase<kTestName__##name>  testCaseInstance__##name; \
    bool XidiTest::TestCase<kTestName__##name>::CanRun(void) const { return (cond); } \
    void XidiTest::TestCase<kTestName__##name>::Run(void) const

/// Recommended way of creating test cases that execute unconditionally.
/// Just provide the test case name.
#define TEST_CASE(name)                     TEST_CASE_CONDITIONAL(name, true)