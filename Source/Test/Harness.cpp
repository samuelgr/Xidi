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
#include <string_view>


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

    int Harness::RunAllTestsInternal(std::wstring_view prefixToMatch)
    {
        std::set<const wchar_t*> failingTests;
        int numExecutedTests = 0;
        int numSkippedTests = 0;

        switch(testCases.size())
        {
        case 0:
            Print(L"\nNo tests defined!\n");
            return -1;

        default:
            PrintFormatted(L"\n%d test%s defined.", testCases.size(), ((1 == testCases.size()) ? L"" : L"s"));
            break;
        }

        if (true == prefixToMatch.empty())
            Print(L"Running all tests.");
        else
            PrintFormatted(L"Running only tests with \"%s\" as a prefix.", prefixToMatch.data());

        Print(L"\n================================================================================");

        for (auto testCaseIterator = testCases.begin(); testCaseIterator != testCases.end(); ++testCaseIterator)
        {
            const bool lastTestCase = (testCaseIterator == --testCases.end());
            const auto& name = testCaseIterator->first;
            const ITestCase* const testCase = testCaseIterator->second;

            if (false == name.starts_with(prefixToMatch))
                continue;

            if (testCase->CanRun())
            {
                PrintFormatted(L"[ %-9s ] %s", L"RUN", name.c_str());

                bool testCasePassed = false;
                try
                {
                    numExecutedTests += 1;

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
        
        if (numSkippedTests > 0)
            PrintFormatted(L"\nFinished running %d test%s (%d skipped).\n", numExecutedTests, ((1 == numExecutedTests) ? L"" : L"s"), numSkippedTests);
        else
            PrintFormatted(L"\nFinished running %d test%s.\n", numExecutedTests, ((1 == numExecutedTests) ? L"" : L"s"));

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
                Print(L"All tests passed!\n");
                break;

            default:
                PrintFormatted(L"%d test%s failed:", numFailingTests, ((1 == numFailingTests) ? L"" : L"s"));
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
int wmain(int argc, const wchar_t* argv[])
{
    return XidiTest::Harness::RunAllTests(((argc > 1) ? argv[1] : L""));
}
