/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Base.cpp
 *      Abstract base class for supported control mapping schemes.
 *      Provides common implementations of most core functionality.
 *****************************************************************************/

#include "Mapper/Base.h"

using namespace XinputControllerDirectInput;
using namespace XinputControllerDirectInput::Mapper;


BOOL CheckAndSetOffsets(BOOL* base, DWORD count)
{
    for (DWORD i = 0; i < count; ++i)
        if (base[i] != FALSE) return FALSE;

    for (DWORD i = 0; i < count; ++i)
        base[i] = TRUE;

    return TRUE;
}

// -------- INSTANCE METHODS ----------------------------------------------- //
// See "Mapper/Base.h" for documentation.

HRESULT Base::ParseApplicationDataFormat(LPCDIDATAFORMAT lpdf)
{
    const TInstanceCount numButtons = NumberOfInstancesOfType(EInstanceType::InstanceTypeButton);
    const TInstanceCount numAxes = NumberOfInstancesOfType(EInstanceType::InstanceTypeAxis);
    const TInstanceCount numPov = NumberOfInstancesOfType(EInstanceType::InstanceTypePov);
    
    BOOL* buttonUsed = new BOOL[numButtons];
    BOOL* axisUsed = new BOOL[numAxes];
    BOOL* povUsed = new BOOL[numPov];
    BOOL* offsetUsed = new BOOL[lpdf->dwDataSize];
    for (TInstanceCount i = 0; i < numButtons; ++i) buttonUsed[i] = FALSE;
    for (TInstanceCount i = 0; i < numAxes; ++i) axisUsed[i] = FALSE;
    for (TInstanceCount i = 0; i < numPov; ++i) povUsed[i] = FALSE;
    for (DWORD i = 0; i < lpdf->dwDataSize; ++i) offsetUsed[i] = FALSE;

    TInstanceIdx nextUnusedButton = 0;
    TInstanceIdx nextUnusedAxis = 0;
    TInstanceIdx nextUnusedPov = 0;


    for (DWORD i = 0; i < lpdf->dwNumObjs; ++i)
    {
        LPDIOBJECTDATAFORMAT dataFormat = &lpdf->rgodf[i];
        const BOOL allowAnyInstance = ((dataFormat->dwType & DIDFT_INSTANCEMASK) == DIDFT_ANYINSTANCE);
        const TInstanceIdx specificInstance = (TInstanceIdx)DIDFT_GETINSTANCE(dataFormat->dwType);
        BOOL invalidParamsDetected = FALSE;

        if ((dataFormat->dwType & DIDFT_ABSAXIS) && (nextUnusedAxis < numAxes))
        {
            // Pick an axis

            if (NULL == dataFormat->pguid)
            {
                // Any axis type allowed

                if (allowAnyInstance)
                {
                    // Any axis instance allowed, just pick the next one
                    axisUsed[nextUnusedAxis] = TRUE;
                    if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                        invalidParamsDetected = TRUE;
                    else
                    {
                        instanceToOffset.insert({ InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeAxis, nextUnusedAxis), dataFormat->dwOfs });
                        offsetToInstance.insert({ dataFormat->dwOfs, InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeAxis, nextUnusedAxis) });
                    }
                }
                else
                {
                    // A specific instance was specified, pick it if it is available

                    if (FALSE == axisUsed[specificInstance])
                    {
                        // Axis is available, use it
                        axisUsed[specificInstance] = TRUE;
                        if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                            invalidParamsDetected = TRUE;
                        else
                        {
                            instanceToOffset.insert({ InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeAxis, specificInstance), dataFormat->dwOfs });
                            offsetToInstance.insert({ dataFormat->dwOfs, InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeAxis, specificInstance) });
                        }
                    }
                    else
                    {
                        // Axis is unavailable, this is an error
                        invalidParamsDetected = TRUE;
                    }
                }
            }
            else
            {
                // Specified an axis type

                if (0 != AxisTypeCount(*dataFormat->pguid))
                {
                    // Axis exists in the mapping presented to the application

                    if (allowAnyInstance)
                    {
                        // Any instance allowed, so find the first of this type that is unused, if any
                        TInstanceIdx instanceIndex = 0;
                        TInstanceIdx axisIndex = AxisInstanceIndex(*dataFormat->pguid, instanceIndex++);
                        while (TRUE == axisUsed[axisIndex] && axisIndex < numAxes) axisIndex = AxisInstanceIndex(*dataFormat->pguid, instanceIndex++);

                        if (axisIndex < numAxes)
                        {
                            // Unused instance found, use it
                            axisUsed[axisIndex] = TRUE;
                            if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                                invalidParamsDetected = TRUE;
                            else
                            {
                                instanceToOffset.insert({ InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeAxis, axisIndex), dataFormat->dwOfs });
                                offsetToInstance.insert({ dataFormat->dwOfs, InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeAxis, axisIndex) });
                            }
                        }
                    }
                    else
                    {
                        // Specific instance required, so check if it is available
                        TInstanceIdx axisIndex = AxisInstanceIndex(*dataFormat->pguid, specificInstance);

                        if (axisIndex < numAxes && FALSE == axisUsed[axisIndex])
                        {
                            // Axis available, use it
                            axisUsed[axisIndex] = TRUE;
                            if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                                invalidParamsDetected = TRUE;
                            else
                            {
                                instanceToOffset.insert({ InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeAxis, axisIndex), dataFormat->dwOfs });
                                offsetToInstance.insert({ dataFormat->dwOfs, InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeAxis, axisIndex) });
                            }
                        }
                        else
                        {
                            // Axis unavailable, this is an error
                            invalidParamsDetected = TRUE;
                        }
                    }

                }
                else
                {
                    // Axis does not exist in the mapping

                    if (!allowAnyInstance)
                    {
                        // Specified an instance of a non-existant axis, this is an error
                        invalidParamsDetected = TRUE;
                    }
                }
            }
        }
        else if ((dataFormat->dwType & DIDFT_PSHBUTTON) && (nextUnusedButton < numButtons))
        {
            // Pick a button

            if (NULL == dataFormat->pguid || IsEqualGUID(GUID_Button, *dataFormat->pguid))
            {
                // Type unspecified or specified as a button

                if (allowAnyInstance)
                {
                    // Any button instance allowed, just pick the next one
                    buttonUsed[nextUnusedButton] = TRUE;
                    if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 1))
                        invalidParamsDetected = TRUE;
                    else
                    {
                        instanceToOffset.insert({ InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeButton, nextUnusedButton), dataFormat->dwOfs });
                        offsetToInstance.insert({ dataFormat->dwOfs, InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeButton, nextUnusedButton) });
                    }
                }
                else
                {
                    // A specific instance was specified, pick it if it is available

                    if (FALSE == buttonUsed[specificInstance])
                    {
                        // Button is available, use it
                        buttonUsed[specificInstance] = TRUE;
                        if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 1))
                            invalidParamsDetected = TRUE;
                        else
                        {
                            instanceToOffset.insert({ InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeButton, specificInstance), dataFormat->dwOfs });
                            offsetToInstance.insert({ dataFormat->dwOfs, InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypeButton, specificInstance) });
                        }
                    }
                    else
                    {
                        // Button is unavailable, this is an error
                        invalidParamsDetected = TRUE;
                    }
                }
            }
            else
            {
                // Type specified as a non-button, this is an error
                invalidParamsDetected = TRUE;
            }

        }
        else if ((dataFormat->dwType & DIDFT_POV) && (nextUnusedPov < numPov))
        {
            // Pick a POV

            if (NULL == dataFormat->pguid || IsEqualGUID(GUID_POV, *dataFormat->pguid))
            {
                // Type unspecified or specified as a POV

                if (allowAnyInstance)
                {
                    // Any POV instance allowed, just pick the next one
                    povUsed[nextUnusedPov] = TRUE;
                    if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                        invalidParamsDetected = TRUE;
                    else
                    {
                        instanceToOffset.insert({ InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypePov, nextUnusedPov), dataFormat->dwOfs });
                        offsetToInstance.insert({ dataFormat->dwOfs, InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypePov, nextUnusedPov) });
                    }
                }
                else
                {
                    // A specific instance was specified, pick it if it is available

                    if (FALSE == povUsed[specificInstance])
                    {
                        // POV is available, use it
                        povUsed[specificInstance] = TRUE;
                        if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                            invalidParamsDetected = TRUE;
                        else
                        {
                            instanceToOffset.insert({ InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypePov, specificInstance), dataFormat->dwOfs });
                            offsetToInstance.insert({ dataFormat->dwOfs, InstanceTypeAndIndexToIdentifier(EInstanceType::InstanceTypePov, specificInstance) });
                        }
                    }
                    else
                    {
                        // Button is unavailable, this is an error
                        invalidParamsDetected = TRUE;
                    }
                }
            }
            else
            {
                // Type specified as a non-button, this is an error
                invalidParamsDetected = TRUE;
            }

        }
        else if (allowAnyInstance)
        {
            // No objects available, but no instance specified, so do not do anything this iteration
        }
        else
        {
            // An instance was specified of an object that is not available, this is an error
            invalidParamsDetected = TRUE;
        }

        // Bail in the event of an error
        if (invalidParamsDetected)
        {
            delete[] buttonUsed;
            delete[] axisUsed;
            delete[] povUsed;
            delete[] offsetUsed;

            return DIERR_INVALIDPARAM;
        }

        // Increment all next-unused indices
        while (TRUE == axisUsed[nextUnusedAxis] && nextUnusedAxis < numAxes) nextUnusedAxis += 1;
        while (TRUE == buttonUsed[nextUnusedButton] && nextUnusedButton < numButtons) nextUnusedButton += 1;
        while (TRUE == povUsed[nextUnusedPov] && nextUnusedPov < numPov) nextUnusedPov += 1;
    }

    delete[] buttonUsed;
    delete[] axisUsed;
    delete[] povUsed;
    delete[] offsetUsed;

    return S_OK;
}
