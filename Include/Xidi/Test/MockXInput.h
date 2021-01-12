/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MockXInput.h
 *   Mock XInput interface that can be used for tests.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "TestCase.h"
#include "XInputInterface.h"

#include <deque>
#include <optional>


namespace XidiTest
{
    /// Mock version the XInput interface, used for test purposes to provide fake XInput data to a virtual controller.
    class MockXInput : public Xidi::IXInput
    {
    public:
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Defines the behavior of a mock method call.
        /// @tparam OutputObjectType Type of method output parameter.
        template <typename OutputObjectType> struct SMethodCallSpec
        {
            DWORD returnCode;                                               ///< Desired return code.
            std::optional<OutputObjectType> maybeOutputObject;              ///< Desired output object. If absent, no object is copied to the output parameter.
            int repeatTimes;                                                ///< Number of times the call should be repeated before it is removed. Zero means the call should happen exactly once.
        };


    private:
        // -------- INSTANCE VARIALBES ------------------------------------- //

        /// Expected user index. All calls will fail if they do not match.
        const DWORD kUserIndex;

        /// Expected behavior for calls to #GetState.
        std::deque<SMethodCallSpec<XINPUT_STATE>> callsGetState;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor.
        /// Requires an XInput user index.
        inline MockXInput(DWORD kUserIndex) : kUserIndex(kUserIndex), callsGetState()
        {
            // Nothing to do here.
        }


    private:
        // -------- CLASS METHODS ------------------------------------------ //

        /// Performs a mock method call.
        /// @tparam OutputObjectType Type of method output parameter.
        /// @param [in] methodName Method name to use when generating error messages.
        /// @param [in,out] callSpecs Ordered list of method call specifications, which is modified during this call as appropriate.
        /// @param [out] outputBuf If the call specification includes an output object, this buffer is filled with the contents of that object.
        /// @return Return code specified in the call specification.
        template <typename OutputObjectType> static DWORD DoMockMethodCall(const wchar_t* methodName, std::deque<SMethodCallSpec<OutputObjectType>>& callSpecs, OutputObjectType* outputBuf)
        {
            if (callSpecs.empty())
                TEST_FAILED_BECAUSE(L"%s: Unexpected method call.", methodName);

            SMethodCallSpec<OutputObjectType>& callSpec = callSpecs.front();
            const DWORD returnCode = callSpec.returnCode;

            if (callSpec.maybeOutputObject.has_value())
            {
                const OutputObjectType outputObject = callSpec.maybeOutputObject.value();
                memcpy(outputBuf, &outputObject, sizeof(outputObject));
            }

            if (0 == callSpec.repeatTimes)
                callSpecs.pop_front();
            else
                callSpec.repeatTimes -= 1;

            return returnCode;
        }


    public:
        // -------- INSTANCE METHODS --------------------------------------- //

        /// Submits an expected call for the #GetState method.
        /// @param [in] callSpec Specifications that describe the desired behavior of the call.
        inline void ExpectCallGetState(const SMethodCallSpec<XINPUT_STATE>& callSpec)
        {
            callsGetState.push_back(callSpec);
        }

        /// Submits multiple expected calls for the #GetState method.
        /// @param [in] callSpec Specifications that describe the desired behavior of the call.
        inline void ExpectCallGetState(std::initializer_list<const SMethodCallSpec<XINPUT_STATE>> callSpecs)
        {
            for (auto& callSpec : callSpecs)
                ExpectCallGetState(callSpec);
        }


        // -------- CONCRETE INSTANCE METHODS ------------------------------ //

        DWORD GetState(DWORD dwUserIndex, XINPUT_STATE* pState) override
        {
            if (kUserIndex != dwUserIndex)
                TEST_FAILED_BECAUSE(L"XInputGetState: User index mismatch (expected %u, got %u).", kUserIndex, dwUserIndex);
            else if (dwUserIndex >= XUSER_MAX_COUNT)
                TEST_FAILED_BECAUSE(L"XInputGetState: User index too large (%u versus maximum %u).", dwUserIndex, XUSER_MAX_COUNT);

            return DoMockMethodCall(L"XInputGetState", callsGetState, pState);
        }
    };
}
