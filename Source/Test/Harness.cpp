/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Harness.cpp
 *   Implementation of the test harness, including program entry point.
 *****************************************************************************/

#include "Harness.h"
#include "TestCase.h"

#include <cstdarg>
#include <cstdio>
#include <map>
#include <windows.h>


namespace XidiTest
{
    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Prints the specified message and appends a newline.
    /// If a debugger is present, outputs a debug string, otherwise writes to standard output.
    /// @param [in] str Message string.
    static void Print(const wchar_t* const str)
    {
        if (IsDebuggerPresent())
        {
            OutputDebugString(str);
            OutputDebugString(L"\n");
        }
        else
        {
            _putws(str);
        }
    }

    /// Formats and prints the specified message and appends a newline.
    /// @param [in] format Message string, possibly with format specifiers.
    /// @param [in] args Variable argument list.
    static void PrintVarArg(const wchar_t* const format, va_list args)
    {
        wchar_t formattedStringBuffer[1024];
        vswprintf_s(formattedStringBuffer, _countof(formattedStringBuffer), format, args);
        Print(formattedStringBuffer);
    }

    /// Formats and prints the specified message.
    /// @param [in] format Message string, possibly with format specifiers.
    static void PrintFormatted(const wchar_t* const format, ...)
    {
        va_list args;
        va_start(args, format);
        PrintVarArg(format, args);
        va_end(args);
    }


    // -------- CLASS METHODS ---------------------------------------------- //
    // See "Harness.h" for documentation.

    Harness& Harness::GetInstance(void)
    {
        static Harness harness;
        return harness;
    }

    // --------

    void Harness::PrintFromTestCase(const ITestCase* const testCase, const wchar_t* const str)
    {
        Print(str);
    }

    // --------

    void Harness::PrintVarArgFromTestCase(const ITestCase* const testCase, const wchar_t* const format, va_list args)
    {
        PrintVarArg(format, args);
    }


    // -------- INSTANCE METHODS ------------------------------------------- //
    // See "Harness.h" for documentation.

    void Harness::RegisterTestCaseInternal(const ITestCase* const testCase, const wchar_t* const name)
    {
        if ((nullptr != name) && ('\0' != name[0]) && (0 == testCases.count(name)))
            testCases[name] = testCase;
    }

    // --------

    int Harness::RunAllTestsInternal(void)
    {
        int numFailingTests = 0;
        int numSkippedTests = 0;

        switch(testCases.size())
        {
        case 0:
            Print(L"\nNo tests defined!\n");
            return -1;

        case 1:
            Print(L"\nRunning 1 test...\n");
            break;

        default:
            PrintFormatted(L"\nRunning %d tests...\n", testCases.size());
            break;
        }

        Print(L"================================================================================");

        for (auto testCaseIterator = testCases.begin(); testCaseIterator != testCases.end(); ++testCaseIterator)
        {
            const bool lastTestCase = (testCaseIterator == --testCases.end());
            const auto& name = testCaseIterator->first;
            const ITestCase* const testCase = testCaseIterator->second;

            if (testCase->CanRun())
            {
                PrintFormatted(L"[ %-9s ] %s", L"RUN", name.c_str());

                const bool testCasePassed = testCase->Run();
                if (true != testCasePassed)
                    numFailingTests += 1;

                PrintFormatted(L"[ %9s ] %s%s", (true == testCasePassed ? L"PASS" : L"FAIL"), name.c_str(), (lastTestCase ? L"" : L"\n"));
            }
            else
            {
                PrintFormatted(L"[  %-8s ] %s%s", L"SKIPPED", name.c_str(), (lastTestCase ? L"" : L"\n"));
                numSkippedTests += 1;
            }
        }

        Print(L"================================================================================");

        if (testCases.size() == numSkippedTests)
        {
            Print(L"All tests skipped.\n");
        }
        else
        {
            switch (numFailingTests)
            {
            case 0:
                if (numSkippedTests > 0)
                    PrintFormatted(L"\nAll tests passed (%d skipped)!\n", numSkippedTests);
                else
                    Print(L"\nAll tests passed!\n");
                break;

            case 1:
                if (numSkippedTests > 0)
                    PrintFormatted(L"\n1 test failed (%d skipped).\n", numSkippedTests);
                else
                    Print(L"\n1 test failed.\n");
                break;

            default:
                if (numSkippedTests > 0)
                    PrintFormatted(L"\n%d tests failed (%d skipped).\n", numFailingTests, numSkippedTests);
                else
                    PrintFormatted(L"\n%d tests failed.\n", numFailingTests);
                break;
            }
        }

        return numFailingTests;
    }
}


// -------- ENTRY POINT ---------------------------------------------------- //

/// Runs all tests cases.
/// @return Number of failing tests (0 means all tests passed).
int main(int argc, const char* argv[])
{
    return XidiTest::Harness::RunAllTests();
}
