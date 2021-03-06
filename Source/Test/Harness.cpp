/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Harness.cpp
 *   Implementation of the test harness, including program entry point.
 *****************************************************************************/

#include "Harness.h"
#include "TestCase.h"
#include "Utilities.h"

#include <map>
#include <set>


namespace XidiTest
{
    // -------- CLASS METHODS ---------------------------------------------- //
    // See "Harness.h" for documentation.

    Harness& Harness::GetInstance(void)
    {
        static Harness harness;
        return harness;
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
        std::set<const wchar_t*> failingTests;
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

                bool testCasePassed = false;
                try
                {
                    testCase->Run();
                    testCasePassed = true;
                }
                catch (TestFailedException)
                {
                    // Nothing to do here.
                }

                if (true != testCasePassed)
                    failingTests.insert(name.c_str());

                PrintFormatted(L"[ %9s ] %s%s", (true == testCasePassed ? L"PASS" : L"FAIL"), name.c_str(), (lastTestCase ? L"" : L"\n"));
            }
            else
            {
                PrintFormatted(L"[  %-8s ] %s%s", L"SKIPPED", name.c_str(), (lastTestCase ? L"" : L"\n"));
                numSkippedTests += 1;
            }
        }

        Print(L"================================================================================");

        const int numFailingTests = (int)failingTests.size();

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
                    Print(L"\n1 test failed:");
                break;

            default:
                if (numSkippedTests > 0)
                    PrintFormatted(L"\n%d tests failed (%d skipped):", numFailingTests, numSkippedTests);
                else
                    PrintFormatted(L"\n%d tests failed:", numFailingTests);
                break;
            }
        }

        if (numFailingTests > 0)
        {
            for (const wchar_t* failingTestName : failingTests)
                PrintFormatted(L"    %s", failingTestName);

            Print(L"\n");
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
