/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * OldGamepad.cpp
 *      Implements a mapper that maps to the button layout of an older
 *      DirectInput-compatible gamepad.
 *****************************************************************************/

#include "Mapper/OldGamepad.h"

using namespace XinputControllerDirectInput;
using namespace XinputControllerDirectInput::Mapper;


DWORD OldGamepad::AxisInstanceIndex(REFGUID axisGUID, DWORD instanceNumber)
{
    // Only one axis of each type exists in this mapping.
    if (0 == instanceNumber)
    {
        if (IsEqualGUID(GUID_XAxis, axisGUID)) return (DWORD)EAxis::AxisX;
        if (IsEqualGUID(GUID_YAxis, axisGUID)) return (DWORD)EAxis::AxisY;
        if (IsEqualGUID(GUID_ZAxis, axisGUID)) return (DWORD)EAxis::AxisZ;
        if (IsEqualGUID(GUID_RzAxis, axisGUID)) return (DWORD)EAxis::AxisRZ;
    }

    return (DWORD)EAxis::AxisCount;
}

BOOL OldGamepad::AxisInstanceExists(REFGUID axisGUID, DWORD instanceNumber)
{
    return (EAxis::AxisCount != AxisInstanceIndex(axisGUID, instanceNumber));
}

DWORD OldGamepad::AxisTypeCount(REFGUID axisGUID)
{
    // Only one axis of each type exists in this mapping.
    if (IsEqualGUID(GUID_XAxis, axisGUID)) return 1;
    if (IsEqualGUID(GUID_YAxis, axisGUID)) return 1;
    if (IsEqualGUID(GUID_ZAxis, axisGUID)) return 1;
    if (IsEqualGUID(GUID_RzAxis, axisGUID)) return 1;

    return 0;
}

BOOL CheckAndSetOffsets(BOOL* base, DWORD count)
{
    for (DWORD i = 0; i < count; ++i)
        if (base[i] != FALSE) return FALSE;

    for (DWORD i = 0; i < count; ++i)
        base[i] = TRUE;

    return TRUE;
}


// -------- CONCRETE INSTANCE METHODS -------------------------------------- //
// See "Mapper.h" for documentation.

HRESULT OldGamepad::ParseApplicationDataFormat(LPCDIDATAFORMAT lpdf)
{
    BOOL* buttonUsed = new BOOL[EButton::ButtonCount];
    BOOL* axisUsed = new BOOL[EAxis::AxisCount];
    BOOL* povUsed = new BOOL[EPov::PovCount];
    BOOL* offsetUsed = new BOOL[lpdf->dwDataSize];
    for (DWORD i = 0; i < EButton::ButtonCount; ++i) buttonUsed[i] = FALSE;
    for (DWORD i = 0; i < EAxis::AxisCount; ++i) axisUsed[i] = FALSE;
    for (DWORD i = 0; i < EPov::PovCount; ++i) povUsed[i] = FALSE;
    for (DWORD i = 0; i < lpdf->dwDataSize; ++i) offsetUsed[i] = FALSE;

    DWORD nextUnusedButton = 0;
    DWORD nextUnusedAxis = 0;
    DWORD nextUnusedPov = 0;


    for (DWORD i = 0; i < lpdf->dwNumObjs; ++i)
    {
        LPDIOBJECTDATAFORMAT dataFormat = &lpdf->rgodf[i];
        const BOOL allowAnyInstance = ((dataFormat->dwType & DIDFT_INSTANCEMASK) == DIDFT_ANYINSTANCE);
        const DWORD specificInstance = DIDFT_GETINSTANCE(dataFormat->dwType);
        BOOL invalidParamsDetected = FALSE;
        
        if ((dataFormat->dwType & DIDFT_ABSAXIS) && (nextUnusedAxis < EAxis::AxisCount))
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
                        DWORD instanceIndex = 0;
                        DWORD axisIndex = AxisInstanceIndex(*dataFormat->pguid, instanceIndex++);
                        while (TRUE == axisUsed[axisIndex] && axisIndex < EAxis::AxisCount) axisIndex = AxisInstanceIndex(*dataFormat->pguid, instanceIndex++);

                        if (axisIndex < EAxis::AxisCount)
                        {
                            // Unused instance found, use it
                            axisUsed[axisIndex] = TRUE;
                            if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                                invalidParamsDetected = TRUE;
                        }
                    }
                    else
                    {
                        // Specific instance required, so check if it is available
                        DWORD axisIndex = AxisInstanceIndex(*dataFormat->pguid, specificInstance);

                        if (axisIndex < EAxis::AxisCount && FALSE == axisUsed[axisIndex])
                        {
                            // Axis available, use it
                            axisUsed[axisIndex] = TRUE;
                            if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                                invalidParamsDetected = TRUE;
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
        else if ((dataFormat->dwType & DIDFT_PSHBUTTON) && (nextUnusedButton < EButton::ButtonCount))
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
        else if ((dataFormat->dwType & DIDFT_POV) && (nextUnusedPov < EPov::PovCount))
        {
            // Pick a POV

            if (NULL == dataFormat->pguid || IsEqualGUID(GUID_POV, *dataFormat->pguid))
            {
                // Type unspecified or specified as a POV

                if (allowAnyInstance)
                {
                    // Any button instance allowed, just pick the next one
                    povUsed[nextUnusedPov] = TRUE;
                    if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                        invalidParamsDetected = TRUE;
                }
                else
                {
                    // A specific instance was specified, pick it if it is available

                    if (FALSE == povUsed[specificInstance])
                    {
                        // Button is available, use it
                        povUsed[specificInstance] = TRUE;
                        if (FALSE == CheckAndSetOffsets(&offsetUsed[dataFormat->dwOfs], 4))
                            invalidParamsDetected = TRUE;
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
        while (TRUE == axisUsed[nextUnusedAxis] && nextUnusedAxis < EAxis::AxisCount) nextUnusedAxis += 1;
        while (TRUE == buttonUsed[nextUnusedButton] && nextUnusedButton < EButton::ButtonCount) nextUnusedButton += 1;
        while (TRUE == povUsed[nextUnusedPov] && nextUnusedPov < EPov::PovCount) nextUnusedPov += 1;
    }
    
    int j = 0;

    delete[] buttonUsed;
    delete[] axisUsed;
    delete[] povUsed;
    delete[] offsetUsed;

    return S_OK;
}
