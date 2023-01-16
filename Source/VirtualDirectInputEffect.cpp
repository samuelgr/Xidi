/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file VirtualDirectInputEffect.cpp
 *   Implementation of an IDirectInputEffect interface wrapper around force
 *   feedback effects that are associated with virtual controllers.
 *****************************************************************************/

#include "ForceFeedbackDevice.h"
#include "ForceFeedbackEffect.h"
#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"
#include "Message.h"
#include "TemporaryBuffer.h"
#include "VirtualDirectInputDevice.h"
#include "VirtualDirectInputEffect.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>


// -------- MACROS --------------------------------------------------------- //

/// Logs a DirectInput interface method invocation and returns.
#define LOG_INVOCATION_AND_RETURN(result, severity) \
    do { \
        const HRESULT kResult = (result); \
        Message::OutputFormatted(severity, L"Invoked %s on force feedback effect with identifier %llu associated with Xidi virtual controller %u, result = 0x%08x.", __FUNCTIONW__ L"()", (unsigned long long)UnderlyingEffect().Identifier(), (1 + associatedDevice.GetVirtualController().GetIdentifier()), kResult); \
        return kResult; \
    } while (false)


namespace Xidi
{
    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Severity to use for dumping the contents of structures to the log.
    static constexpr Message::ESeverity kDumpSeverity = Message::ESeverity::Debug;


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Internal implementation of downloading a force feedback effect to a force feedback device.
    /// Called by interface methods that require this functionality.
    /// @param [in] effect Effect to be downloaded.
    /// @param [in] device Target device.
    /// @return DirectInput return code indicating the result of the operation.
    static HRESULT DownloadEffectToDevice(const Controller::ForceFeedback::Effect& effect, Controller::ForceFeedback::Device& device)
    {
        if (false == effect.IsCompletelyDefined())
            return DIERR_INCOMPLETEEFFECT;

        if (false == device.AddOrUpdateEffect(effect))
            return DIERR_DEVICEFULL;

        return DI_OK;
    }

    /// Parses the specified flags, which would normally be passed to SetParameters, and extracts individual strings for each flag that is present.
    /// @param [in] dwFlags Flags to parse.
    /// @return String representation of the flags that are set.
    static TemporaryString ParameterTopLevelFlagsToString(DWORD dwFlags)
    {
        constexpr std::wstring_view kFlagSeparator = L" | ";
        TemporaryString flagsString;

        if (0 != (dwFlags & DIEP_NODOWNLOAD))
            flagsString << L"DIEP_NODOWNLOAD" << kFlagSeparator;
        if (0 != (dwFlags & DIEP_NORESTART))
            flagsString << L"DIEP_NORESTART" << kFlagSeparator;
        if (0 != (dwFlags & DIEP_START))
            flagsString << L"DIEP_START" << kFlagSeparator;

        if (DIEP_ALLPARAMS == (dwFlags & DIEP_ALLPARAMS))
        {
            flagsString << L"DIEP_ALLPARAMS" << kFlagSeparator;
        }
        else if (DIEP_ALLPARAMS_DX5 == (dwFlags & DIEP_ALLPARAMS_DX5))
        {
            flagsString << L"DIEP_ALLPARAMS_DX5" << kFlagSeparator;
        }
        else
        {
            if (0 != (dwFlags & DIEP_AXES))
                flagsString << L"DIEP_AXES" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_DIRECTION))
                flagsString << L"DIEP_DIRECTION" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_DURATION))
                flagsString << L"DIEP_DURATION" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_ENVELOPE))
                flagsString << L"DIEP_ENVELOPE" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_GAIN))
                flagsString << L"DIEP_GAIN" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_SAMPLEPERIOD))
                flagsString << L"DIEP_SAMPLEPERIOD" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_STARTDELAY))
                flagsString << L"DIEP_STARTDELAY" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_TRIGGERBUTTON))
                flagsString << L"DIEP_TRIGGERBUTTON" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_TRIGGERREPEATINTERVAL))
                flagsString << L"DIEP_TRIGGERREPEATINTERVAL" << kFlagSeparator;
            if (0 != (dwFlags & DIEP_TYPESPECIFICPARAMS))
                flagsString << L"DIEP_TYPESPECIFICPARAMS" << kFlagSeparator;
        }

        if (true == flagsString.Empty())
            return flagsString << L"none" << kFlagSeparator;

        flagsString.RemoveSuffix((unsigned int)kFlagSeparator.length());
        return flagsString;
    }

    /// Attempts to recognize the size of a parameter structure and returns a string representing what was recognized.
    /// @param [in] dwSize Size of the structure, as extracted from the relevant field.
    /// @return String representation of the recognized structure.
    static const wchar_t* ParameterStructSizeToString(DWORD dwSize)
    {
        switch (dwSize)
        {
        case sizeof(DIEFFECT):
            return L"sizeof(DIEFFECT)";

        case sizeof(DIEFFECT_DX5):
            return L"sizeof(DIEFFECT_DX5)";

        default:
            return L"unknown";
        }
    }

    /// Parses the specified flags, which would normally be present inside the dwFlags member of DIEFFECT, and extracts individual strings for each flag that is present.
    /// @param [in] dwFlags Flags to parse.
    /// @return String representation of the flags that are set.
    static const TemporaryString ParameterStructFlagsToString(DWORD dwFlags)
    {
        constexpr std::wstring_view kFlagSeparator = L" | ";
        TemporaryString flagsString;

        if (0 != (dwFlags & DIEFF_CARTESIAN))
            flagsString << L"DIEFF_CARTESIAN" << kFlagSeparator;
        if (0 != (dwFlags & DIEFF_POLAR))
            flagsString << L"DIEFF_POLAR" << kFlagSeparator;
        if (0 != (dwFlags & DIEFF_SPHERICAL))
            flagsString << L"DIEFF_SPHERICAL" << kFlagSeparator;
        if (0 != (dwFlags & DIEFF_OBJECTIDS))
            flagsString << L"DIEFF_OBJECTIDS" << kFlagSeparator;
        if (0 != (dwFlags & DIEFF_OBJECTOFFSETS))
            flagsString << L"DIEFF_OBJECTOFFSETS" << kFlagSeparator;

        if (true == flagsString.Empty())
            return flagsString << L"none" << kFlagSeparator;

        flagsString.RemoveSuffix((unsigned int)kFlagSeparator.length());
        return flagsString;
    }

    /// Selects the coordinate system that should be used to represent the coordinates set in the specified direction vector, subject to the specified flags.
    /// @param [in] directionVector Vector from which coordinates are to be extracted.
    /// @param [in] dwFlags Flags specifying the allowed coordinate systems using DirectInput constants.
    /// @return Selected coordinate system if it is valid. An example of an invalid situation is polar coordinates specified by the flags but the direction vector contains only one axis or more than two axes.
    static std::optional<Controller::ForceFeedback::ECoordinateSystem> PickCoordinateSystem(const Controller::ForceFeedback::DirectionVector& directionVector, DWORD dwFlags)
    {
        if (1 == directionVector.GetNumAxes())
        {
            // Only Cartesian coordinates are valid with a single axis.
            if (0 == (dwFlags & DIEFF_CARTESIAN))
                return std::nullopt;
        }
        else if (2 != directionVector.GetNumAxes())
        {
            // Polar coordinates are only valid when there are two axes present.
            if (DIEFF_POLAR == (dwFlags & (DIEFF_CARTESIAN | DIEFF_POLAR | DIEFF_SPHERICAL)))
                return std::nullopt;
        }

        // Match the original coordinate system if possible.
        // The output would be exactly what was originally supplied as input when setting parameters.
        switch (directionVector.GetOriginalCoordinateSystem())
        {
        case Controller::ForceFeedback::ECoordinateSystem::Cartesian:
            if (0 != (dwFlags & DIEFF_CARTESIAN))
                return Controller::ForceFeedback::ECoordinateSystem::Cartesian;
            break;

        case Controller::ForceFeedback::ECoordinateSystem::Polar:
            if (0 != (dwFlags & DIEFF_POLAR))
                return Controller::ForceFeedback::ECoordinateSystem::Polar;
            break;

        case Controller::ForceFeedback::ECoordinateSystem::Spherical:
            if (0 != (dwFlags & DIEFF_SPHERICAL))
                return Controller::ForceFeedback::ECoordinateSystem::Spherical;
            break;
        }

        // Try other coordinate systems in Xidi-preferred order.
        if (0 != (dwFlags & DIEFF_SPHERICAL))
            return Controller::ForceFeedback::ECoordinateSystem::Spherical;
        else if (0 != (dwFlags & DIEFF_POLAR))
            return Controller::ForceFeedback::ECoordinateSystem::Polar;
        else if (0 != (dwFlags & DIEFF_CARTESIAN))
            return Controller::ForceFeedback::ECoordinateSystem::Cartesian;

        return std::nullopt;
    }


    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "VirtualDirectInputEffect.h" for documentation.

    template <ECharMode charMode> VirtualDirectInputEffect<charMode>::VirtualDirectInputEffect(VirtualDirectInputDevice<charMode>& associatedDevice, const Controller::ForceFeedback::Effect& effect, const GUID& effectGuid) : associatedDevice(associatedDevice), effect(effect.Clone()), effectGuid(effectGuid), refCount(1)
    {
        associatedDevice.AddRef();
        associatedDevice.ForceFeedbackEffectRegister((void*)this);
    }

    // --------

    template <ECharMode charMode> VirtualDirectInputEffect<charMode>::~VirtualDirectInputEffect(void)
    {
        // If the effect represented by this object is already downloaded to a device then it must be removed from that device.
        Controller::ForceFeedback::Device* const forceFeedbackDevice = associatedDevice.GetVirtualController().ForceFeedbackGetDevice();
        if (nullptr != forceFeedbackDevice)
            forceFeedbackDevice->RemoveEffect(effect->Identifier());
        
        associatedDevice.ForceFeedbackEffectUnregister((void*)this);
        associatedDevice.Release();
    }

    // -------- INSTANCE METHODS ------------------------------------------- //
    // See "VirtualDirectInputEffect.h" for documentation.

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::DownloadInternal(void)
    {
        Controller::ForceFeedback::Device* const forceFeedbackDevice = associatedDevice.AutoAcquireAndGetForceFeedbackDevice();
        if (nullptr == forceFeedbackDevice)
            return DIERR_NOTEXCLUSIVEACQUIRED;

        return DownloadEffectToDevice(*effect, *forceFeedbackDevice);
    }

    // --------

    template <ECharMode charMode> void VirtualDirectInputEffect<charMode>::DumpEffectParameters(LPCDIEFFECT peff, DWORD dwFlags) const
    {
        if (Message::WillOutputMessageOfSeverity(kDumpSeverity))
        {
            Message::Output(kDumpSeverity, L"Begin dump of effect parameters.");

            Message::Output(kDumpSeverity, L"  Control:");
            Message::OutputFormatted(kDumpSeverity, L"    flags = 0x%08x (%s)", dwFlags, ParameterTopLevelFlagsToString(dwFlags).AsCString());

            Message::Output(kDumpSeverity, L"  Basics:");
            if (nullptr == peff)
            {
                Message::Output(kDumpSeverity, L"    (nullptr)");
            }
            else
            {
                Message::OutputFormatted(kDumpSeverity, L"    dwSize = %u (%s)", peff->dwSize, ParameterStructSizeToString(peff->dwSize));
                Message::OutputFormatted(kDumpSeverity, L"    dwFlags = 0x%08x (%s)", peff->dwFlags, ParameterStructFlagsToString(peff->dwFlags).AsCString());

                if (0 != (dwFlags & DIEP_DURATION))
                {
                    if (INFINITE == peff->dwDuration)
                        Message::OutputFormatted(kDumpSeverity, L"    dwDuration = %u (INFINITE)", peff->dwDuration);
                    else
                        Message::OutputFormatted(kDumpSeverity, L"    dwDuration = %u", peff->dwDuration);
                }

                if (0 != (dwFlags & DIEP_SAMPLEPERIOD))
                    Message::OutputFormatted(kDumpSeverity, L"    dwSamplePeriod = %u", peff->dwSamplePeriod);

                if (0 != (dwFlags & DIEP_GAIN))
                    Message::OutputFormatted(kDumpSeverity, L"    dwGain = %u", peff->dwGain);

                if (0 != (dwFlags & DIEP_STARTDELAY))
                    Message::OutputFormatted(kDumpSeverity, L"    dwStartDelay = %u", peff->dwStartDelay);

                if (0 != (dwFlags & DIEP_TRIGGERBUTTON))
                {
                    if (DIEB_NOTRIGGER == peff->dwTriggerButton)
                        Message::OutputFormatted(kDumpSeverity, L"    dwTriggerButton = %u (DIEB_NOTRIGGER)", peff->dwTriggerButton);
                    else
                        Message::OutputFormatted(kDumpSeverity, L"    dwTriggerButton = %u", peff->dwTriggerButton);
                }

                if (0 != (dwFlags & DIEP_TRIGGERREPEATINTERVAL))
                    Message::OutputFormatted(kDumpSeverity, L"    dwTriggerRepeatInterval = %u", peff->dwTriggerRepeatInterval);

                if (0 != (dwFlags & DIEP_AXES))
                {
                    Message::Output(kDumpSeverity, L"  Axes:");
                    Message::OutputFormatted(kDumpSeverity, L"    cAxes = %u", peff->cAxes);

                    if (nullptr == peff->rgdwAxes)
                    {
                        Message::Output(kDumpSeverity, L"    rgdxAxes = (nullptr)");
                    }
                    else
                    {
                        for (DWORD i = 0; i < peff->cAxes; ++i)
                        {
                            std::optional<Controller::SElementIdentifier> maybeAxisElement = std::nullopt;

                            switch (peff->dwFlags & (DIEFF_OBJECTIDS | DIEFF_OBJECTOFFSETS))
                            {
                            case DIEFF_OBJECTIDS:
                                maybeAxisElement = associatedDevice.IdentifyElement(peff->rgdwAxes[i], DIPH_BYID);
                                break;

                            case DIEFF_OBJECTOFFSETS:
                                maybeAxisElement = associatedDevice.IdentifyElement(peff->rgdwAxes[i], DIPH_BYOFFSET);
                                break;
                            }

                            if (true == maybeAxisElement.has_value())
                            {
                                TemporaryBuffer<wchar_t> axisElementString;
                                VirtualDirectInputDevice<ECharMode::W>::ElementToString(maybeAxisElement.value(), axisElementString.Data(), axisElementString.Capacity());
                                Message::OutputFormatted(kDumpSeverity, L"    rgdwAxes[%2u] = 0x%04x (%s)", i, peff->rgdwAxes[i], axisElementString.Data());
                            }
                            else
                            {
                                Message::OutputFormatted(kDumpSeverity, L"    rgdwAxes[%2u] = 0x%04x (unable to identify)", i, peff->rgdwAxes[i]);
                            }
                        }
                    }
                }

                if (0 != (dwFlags & DIEP_DIRECTION))
                {
                    Message::Output(kDumpSeverity, L"  Direction:");
                    Message::OutputFormatted(kDumpSeverity, L"    cAxes = %u", peff->cAxes);

                    if (nullptr == peff->rglDirection)
                    {
                        Message::Output(kDumpSeverity, L"    rglDirection = (nullptr)");
                    }
                    else
                    {
                        for (DWORD i = 0; i < peff->cAxes; ++i)
                        {
                            Message::OutputFormatted(kDumpSeverity, L"    rglDirection[%2u] = %u", i, peff->rglDirection[i]);
                        }
                    }
                }

                if (0 != (dwFlags & DIEP_ENVELOPE))
                {
                    Message::Output(kDumpSeverity, L"  Envelope:");
                    if (nullptr == peff->lpEnvelope)
                    {
                        Message::Output(kDumpSeverity, L"    (nullptr)");
                    }
                    else
                    {
                        const DIENVELOPE* const envelope = (const DIENVELOPE*)peff->lpEnvelope;
                        Message::OutputFormatted(kDumpSeverity, L"    lpEnvelope->dwSize = %u (%s)", envelope->dwSize, ((sizeof(DIENVELOPE) == envelope->dwSize) ? L"sizeof(DIENVELOPE)" : L"unknown"));
                        Message::OutputFormatted(kDumpSeverity, L"    lpEnvelope->dwAttackLevel = %u", envelope->dwAttackLevel);
                        Message::OutputFormatted(kDumpSeverity, L"    lpEnvelope->dwAttackTime = %u", envelope->dwAttackTime);
                        Message::OutputFormatted(kDumpSeverity, L"    lpEnvelope->dwFadeLevel = %u", envelope->dwFadeLevel);
                        Message::OutputFormatted(kDumpSeverity, L"    lpEnvelope->dwFadeTime = %u", envelope->dwFadeTime);
                    }
                }

                if (0 != (dwFlags & DIEP_TYPESPECIFICPARAMS))
                {
                    Message::Output(kDumpSeverity, L"  Type-Specific:");

                    if (nullptr == peff->lpvTypeSpecificParams)
                        Message::Output(kDumpSeverity, L"    (nullptr)");
                    else
                        DumpTypeSpecificParameters(peff);
                }
            }

            Message::Output(kDumpSeverity, L"End dump of effect parameters.");
        }
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::SetParametersInternal(LPCDIEFFECT peff, DWORD dwFlags, std::optional<Controller::ForceFeedback::TEffectTimeMs> timestamp)
    {
        DumpEffectParameters(peff, dwFlags);

        if (nullptr == peff)
            return DIERR_INVALIDPARAM;
        
        // These flags control the behavior of this method if all parameters are updated successfully.
        // Per DirectInput documentation, at most one of them is allowed to be passed.
        switch (dwFlags & (DIEP_NODOWNLOAD | DIEP_NORESTART | DIEP_START))
        {
        case 0:
        case DIEP_NODOWNLOAD:
        case DIEP_NORESTART:
        case DIEP_START:
            break;

        default:
            return DIERR_INVALIDPARAM;
        }

        // This cloned effect will receive all the parameter updates and will be synced back to the original effect once all parameter values are accepted.
        // Doing this means that an invalid value for a parameter means the original effect remains untouched.
        std::unique_ptr<Controller::ForceFeedback::Effect> updatedEffect;

        if (0 != (dwFlags & DIEP_TYPESPECIFICPARAMS))
        {
            if (nullptr == peff->lpvTypeSpecificParams)
                return DIERR_INVALIDPARAM;

            updatedEffect = CloneAndSetTypeSpecificParameters(peff);
            if (nullptr == updatedEffect)
                return DIERR_INVALIDPARAM;
        }
        else
        {
            updatedEffect = effect->Clone();
        }

        switch (peff->dwSize)
        {
            case sizeof(DIEFFECT) :
                // These parameters are present in the new version of the structure but not in the old.
                if (0 != (dwFlags & DIEP_STARTDELAY))
                {
                    if (false == updatedEffect->SetStartDelay(ConvertTimeFromDirectInput(peff->dwStartDelay)))
                        return DIERR_INVALIDPARAM;
                }
                break;

                case sizeof(DIEFFECT_DX5) :
                    // No parameters are present in the old version of the structure but removed from the new.
                    break;

                default:
                    return DIERR_INVALIDPARAM;
        }

        if (0 != (dwFlags & DIEP_AXES))
        {
            if (peff->cAxes > Controller::ForceFeedback::kEffectAxesMaximumNumber)
                return DIERR_INVALIDPARAM;

            if (nullptr == peff->rgdwAxes)
                return DIERR_INVALIDPARAM;

            Controller::ForceFeedback::SAssociatedAxes newAssociatedAxes = {.count = (int)peff->cAxes};
            if ((size_t)newAssociatedAxes.count > newAssociatedAxes.type.size())
                return DIERR_INVALIDPARAM;

            DWORD identifyElementMethod = 0;

            switch (peff->dwFlags & (DIEFF_OBJECTIDS | DIEFF_OBJECTOFFSETS))
            {
            case DIEFF_OBJECTIDS:
                identifyElementMethod = DIPH_BYID;
                break;

            case DIEFF_OBJECTOFFSETS:
                identifyElementMethod = DIPH_BYOFFSET;
                break;

            default:
                // It is an error if the caller does not specify exactly one specific way of identifying axis objects.
                return DIERR_INVALIDPARAM;
            }

            for (int i = 0; i < newAssociatedAxes.count; ++i)
            {
                const std::optional<Controller::SElementIdentifier> kMaybeElement = associatedDevice.IdentifyElement(peff->rgdwAxes[i], identifyElementMethod);
                if (false == kMaybeElement.has_value())
                    return DIERR_INVALIDPARAM;
                
                const Controller::SElementIdentifier kElement = kMaybeElement.value();
                if (Controller::EElementType::Axis != kElement.type)
                    return DIERR_INVALIDPARAM;

                if (false == associatedDevice.GetVirtualController().GetCapabilities().ForceFeedbackIsSupportedForAxis(kElement.axis))
                    return DIERR_INVALIDPARAM;

                newAssociatedAxes.type[i] = kElement.axis;
            }

            if (false == updatedEffect->SetAssociatedAxes(newAssociatedAxes))
                return DIERR_INVALIDPARAM;
        }

        if (0 != (dwFlags & DIEP_DIRECTION))
        {
            if (peff->cAxes > Controller::ForceFeedback::kEffectAxesMaximumNumber)
                return DIERR_INVALIDPARAM;

            if (nullptr == peff->rglDirection)
                return DIERR_INVALIDPARAM;

            Controller::ForceFeedback::TEffectValue coordinates[Controller::ForceFeedback::kEffectAxesMaximumNumber] = {};
            int numCoordinates = peff->cAxes;

            for (int i = 0; i < numCoordinates; ++i)
                coordinates[i] = (Controller::ForceFeedback::TEffectValue)peff->rglDirection[i];

            bool coordinateSetResult = false;
            switch (peff->dwFlags & (DIEFF_CARTESIAN | DIEFF_POLAR | DIEFF_SPHERICAL))
            {
            case DIEFF_CARTESIAN:
                coordinateSetResult = updatedEffect->Direction().SetDirectionUsingCartesian(coordinates, numCoordinates);
                break;

            case DIEFF_POLAR:
                coordinateSetResult = updatedEffect->Direction().SetDirectionUsingPolar(coordinates, numCoordinates - 1);
                break;

            case DIEFF_SPHERICAL:
                coordinateSetResult = updatedEffect->Direction().SetDirectionUsingSpherical(coordinates, numCoordinates - 1);
                break;

            default:
                // It is an error if the caller does not specify exactly one coordinate system.
                return DIERR_INVALIDPARAM;
            }

            if (true != coordinateSetResult)
                return DIERR_INVALIDPARAM;
        }

        if (0 != (dwFlags & DIEP_DURATION))
        {
            if (false == updatedEffect->SetDuration(ConvertTimeFromDirectInput(peff->dwDuration)))
                return DIERR_INVALIDPARAM;
        }

        if (0 != (dwFlags & DIEP_ENVELOPE))
        {
            if (nullptr == peff->lpEnvelope)
            {
                updatedEffect->ClearEnvelope();
            }
            else
            {
                if (sizeof(DIENVELOPE) != peff->lpEnvelope->dwSize)
                    return DIERR_INVALIDPARAM;

                const Controller::ForceFeedback::SEnvelope kNewEnvelope = {
                    .attackTime = ConvertTimeFromDirectInput(peff->lpEnvelope->dwAttackTime),
                    .attackLevel = (Controller::ForceFeedback::TEffectValue)peff->lpEnvelope->dwAttackLevel,
                    .fadeTime = ConvertTimeFromDirectInput(peff->lpEnvelope->dwFadeTime),
                    .fadeLevel = (Controller::ForceFeedback::TEffectValue)peff->lpEnvelope->dwFadeLevel
                };

                if (false == updatedEffect->SetEnvelope(kNewEnvelope))
                    return DIERR_INVALIDPARAM;
            }
        }

        if (0 != (dwFlags & DIEP_GAIN))
        {
            if (false == updatedEffect->SetGain((Controller::ForceFeedback::TEffectValue)peff->dwGain))
                return DIERR_INVALIDPARAM;
        }

        if (0 != (dwFlags & DIEP_SAMPLEPERIOD))
        {
            if (false == updatedEffect->SetSamplePeriod(ConvertTimeFromDirectInput(peff->dwSamplePeriod)))
                return DIERR_INVALIDPARAM;
        }

        // Final sync operation is expected to succeed.
        if (false == effect->SyncParametersFrom(*updatedEffect))
        {
            Message::OutputFormatted(Message::ESeverity::Error, L"Internal error while syncing new parameters for a force feedback effect associated with Xidi virtual controller %u.", (1 + associatedDevice.GetVirtualController().GetIdentifier()));
            return DIERR_GENERIC;
        }

        // Destroying this object now means that any future references to it will trigger crashes during testing.
        updatedEffect = nullptr;

        // At this point parameter updates were successful. What happens next depends on the flag values.
        // The effect could either be downloaded, downloaded and (re)started, or none of these.
        if (0 != (dwFlags & DIEP_NODOWNLOAD))
        {
            return DI_DOWNLOADSKIPPED;
        }
        else
        {
            Controller::ForceFeedback::Device* const forceFeedbackDevice = associatedDevice.AutoAcquireAndGetForceFeedbackDevice();
            if (nullptr == forceFeedbackDevice)
            {
                // It is not an error if the physical device has not been, or cannot be, acquired in exclusive mode.
                // In that case, the download operation is skipped but parameters are still updated.
                return DI_DOWNLOADSKIPPED;
            }

            // If the download operation fails the parameters have still been updated but the caller needs to be provided the reason for the failure.
            const HRESULT kDownloadResult = DownloadEffectToDevice(*effect, *forceFeedbackDevice);
            if (DI_OK != kDownloadResult)
                return kDownloadResult;
        }

        // Default behavior is to update an effect without changing its play state. Playing effects are updated on-the-fly, and non-playing effects are not started.
        if (0 != (dwFlags & DIEP_START))
        {
            // Getting to this point means the effect exists on the device.
            // Starting or restarting the effect requires that the device be acquired in exclusive mode, although since the download succeeded this should be the case already.
            Controller::ForceFeedback::Device* const forceFeedbackDevice = associatedDevice.GetVirtualController().ForceFeedbackGetDevice();
            if (nullptr == forceFeedbackDevice)
            {
                // This should never happen. It means an effect exists on the device and yet the device is somehow not acquired in exclusive mode.
                Message::OutputFormatted(Message::ESeverity::Error, L"Internal error while attempting to start or restart a force feedback effect after setting its parameters on Xidi virtual controller %u.", (1 + associatedDevice.GetVirtualController().GetIdentifier()));
                return DIERR_GENERIC;
            }

            forceFeedbackDevice->StopEffect(effect->Identifier());

            if (false == forceFeedbackDevice->StartEffect(effect->Identifier(), 1, timestamp))
            {
                // This should never happen. It means an effect that in theory should be downloaded and ready to play is somehow unable to be started.
                Message::OutputFormatted(Message::ESeverity::Error, L"Internal error while attempting to start or restart a force feedback effect after setting its parameters on Xidi virtual controller %u.", (1 + associatedDevice.GetVirtualController().GetIdentifier()));
                return DIERR_GENERIC;
            }
        }

        return DI_OK;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::StartInternal(DWORD dwIterations, DWORD dwFlags, std::optional<Controller::ForceFeedback::TEffectTimeMs> timestamp)
    {
        if (0 == dwIterations)
            return DIERR_INVALIDPARAM;

        Controller::ForceFeedback::Device* const forceFeedbackDevice = associatedDevice.AutoAcquireAndGetForceFeedbackDevice();
        if (nullptr == forceFeedbackDevice)
            return DIERR_NOTEXCLUSIVEACQUIRED;

        if (0 != (dwFlags & DIES_NODOWNLOAD))
        {
            // The download operation was skipped by the caller.
            // If the effect does not already exist on the device then the effect cannot be played.
            if (false == forceFeedbackDevice->IsEffectOnDevice(effect->Identifier()))
                return DIERR_INVALIDPARAM;
        }
        else
        {
            // The download operation was not skipped by the caller.
            // If the effect exists on the device its parameters will get updated, otherwise the effect will be downloaded.
            // If for some reason the download attempt fails then the effect cannot be played.
            const HRESULT kDownloadResult = DownloadEffectToDevice(*effect, *forceFeedbackDevice);
            if (DI_OK != kDownloadResult)
                return kDownloadResult;
        }

        if (0 != (dwFlags & DIES_SOLO))
            forceFeedbackDevice->StopAllEffects();
        else
            forceFeedbackDevice->StopEffect(effect->Identifier());

        if (false == forceFeedbackDevice->StartEffect(effect->Identifier(), (unsigned int)dwIterations, timestamp))
        {
            Message::OutputFormatted(Message::ESeverity::Error, L"Internal error while starting a force feedback effect associated with Xidi virtual controller %u.", (1 + associatedDevice.GetVirtualController().GetIdentifier()));
            return DIERR_GENERIC;
        }

        return DI_OK;
    }


    // -------- CONCRETE INSTANCE METHODS ---------------------------------- //
    // See "VirtualDirectInputEffect.h" for documentation.

    template <ECharMode charMode> void VirtualDirectInputEffect<charMode>::DumpTypeSpecificParameters(LPCDIEFFECT peff) const
    {
        Message::OutputFormatted(kDumpSeverity, L"    cbTypeSpecificParams = %u (unknown)", peff->cbTypeSpecificParams);
        Message::OutputFormatted(kDumpSeverity, L"    lpvTypeSpecificParams = (%s)", ((nullptr == peff->lpvTypeSpecificParams) ? L"nullptr" : L"present"));
    }

    // --------

    template <ECharMode charMode> void ConstantForceDirectInputEffect<charMode>::DumpTypeSpecificParameters(LPCDIEFFECT peff) const
    {
        if (sizeof(DICONSTANTFORCE) == peff->cbTypeSpecificParams)
        {
            const DICONSTANTFORCE* const typeSpecificParams = (const DICONSTANTFORCE*)peff->lpvTypeSpecificParams;
            Message::OutputFormatted(kDumpSeverity, L"    cbTypeSpecificParams = %u (sizeof(DICONSTANTFORCE))", peff->cbTypeSpecificParams);
            Message::OutputFormatted(kDumpSeverity, L"    lpvTypeSpecificParams->lMagnitude = %ld", typeSpecificParams->lMagnitude);
        }
        else
        {
            VirtualDirectInputEffect<charMode>::DumpTypeSpecificParameters(peff);
        }
    }

    // --------

    template <ECharMode charMode> void PeriodicDirectInputEffect<charMode>::DumpTypeSpecificParameters(LPCDIEFFECT peff) const
    {
        if (sizeof(DIPERIODIC) == peff->cbTypeSpecificParams)
        {
            const DIPERIODIC* const typeSpecificParams = (const DIPERIODIC*)peff->lpvTypeSpecificParams;
            Message::OutputFormatted(kDumpSeverity, L"    cbTypeSpecificParams = %u (sizeof(DIPERIODIC))", peff->cbTypeSpecificParams);
            Message::OutputFormatted(kDumpSeverity, L"    lpvTypeSpecificParams->dwMagnitude = %u", typeSpecificParams->dwMagnitude);
            Message::OutputFormatted(kDumpSeverity, L"    lpvTypeSpecificParams->lOffset = %ld", typeSpecificParams->lOffset);
            Message::OutputFormatted(kDumpSeverity, L"    lpvTypeSpecificParams->dwPhase = %u", typeSpecificParams->dwPhase);
            Message::OutputFormatted(kDumpSeverity, L"    lpvTypeSpecificParams->dwPeriod = %u", typeSpecificParams->dwPeriod);
        }
        else
        {
            VirtualDirectInputEffect<charMode>::DumpTypeSpecificParameters(peff);
        }
    }

    // --------

    template <ECharMode charMode> void RampForceDirectInputEffect<charMode>::DumpTypeSpecificParameters(LPCDIEFFECT peff) const
    {
        if (sizeof(DIRAMPFORCE) == peff->cbTypeSpecificParams)
        {
            const DIRAMPFORCE* const typeSpecificParams = (const DIRAMPFORCE*)peff->lpvTypeSpecificParams;
            Message::OutputFormatted(kDumpSeverity, L"    cbTypeSpecificParams = %u (sizeof(DIRAMPFORCE))", peff->cbTypeSpecificParams);
            Message::OutputFormatted(kDumpSeverity, L"    lpvTypeSpecificParams->lStart = %ld", typeSpecificParams->lStart);
            Message::OutputFormatted(kDumpSeverity, L"    lpvTypeSpecificParams->lEnd = %ld", typeSpecificParams->lEnd);
        }
        else
        {
            VirtualDirectInputEffect<charMode>::DumpTypeSpecificParameters(peff);
        }
    }


    // -------- METHODS: IUnknown ------------------------------------------ //
    // See IUnknown documentation for more information.

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::QueryInterface(REFIID riid, LPVOID* ppvObj)
    {
        if (nullptr == ppvObj)
            return E_POINTER;

        bool validInterfaceRequested = false;

        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputEffect))
            validInterfaceRequested = true;

        if (true == validInterfaceRequested)
        {
            AddRef();
            *ppvObj = this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    // --------

    template <ECharMode charMode> ULONG VirtualDirectInputEffect<charMode>::AddRef(void)
    {
        return ++refCount;
    }

    // --------

    template <ECharMode charMode> ULONG VirtualDirectInputEffect<charMode>::Release(void)
    {
        const unsigned long numRemainingRefs = --refCount;

        if (0 == numRemainingRefs)
            delete this;

        return (ULONG)numRemainingRefs;
    }


    // -------- METHODS: IDirectInputEffect -------------------------------- //
    // See DirectInput documentation for more information.

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
    {
        // Not required for Xidi virtual force feedback effects as they are implemented now.
        // However, this method is needed for creating IDirectInputDevice objects via COM.

        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::GetEffectGuid(LPGUID pguid)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        if (nullptr == pguid)
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        *pguid = effectGuid;
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::GetParameters(LPDIEFFECT peff, DWORD dwFlags)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        if (nullptr == peff)
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        switch (peff->dwSize)
        {
            case sizeof(DIEFFECT) :
                // These parameters are present in the new version of the structure but not in the old.
                if (0 != (dwFlags & DIEP_STARTDELAY))
                    peff->dwStartDelay = ConvertTimeToDirectInput(effect->GetStartDelay());
                break;

            case sizeof(DIEFFECT_DX5):
                // No parameters are present in the old version of the structure but removed from the new.
                break;

            default:
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
        }

        bool axesInsufficientBuffer = false;
        if (0 != (dwFlags & DIEP_AXES))
        {
            if (false == effect->HasAssociatedAxes())
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

            const Controller::ForceFeedback::SAssociatedAxes& kAssociatedAxes = effect->GetAssociatedAxes().value();
            if (peff->cAxes < (DWORD)kAssociatedAxes.count)
            {
                peff->cAxes = (DWORD)kAssociatedAxes.count;
                axesInsufficientBuffer = true;
            }
            else
            {
                if (nullptr == peff->rgdwAxes)
                    LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

                switch (peff->dwFlags & (DIEFF_OBJECTIDS | DIEFF_OBJECTOFFSETS))
                {
                case DIEFF_OBJECTIDS:
                    for (int i = 0; i < kAssociatedAxes.count; ++i)
                    {
                        const std::optional<DWORD> kMaybeObjectId = associatedDevice.IdentifyObjectById({.type = Controller::EElementType::Axis, .axis = kAssociatedAxes.type[i]});
                        if (false == kMaybeObjectId.has_value())
                        {
                            // This should never happen. It means an axis object was successfully set on a force feedback effect but it could not be mapped back to its object ID.
                            Message::OutputFormatted(Message::ESeverity::Error, L"Internal error while mapping force feedback axes to object IDs on Xidi virtual controller %u.", (1 + associatedDevice.GetVirtualController().GetIdentifier()));
                            LOG_INVOCATION_AND_RETURN(DIERR_GENERIC, kMethodSeverity);
                        }

                        peff->rgdwAxes[i] = kMaybeObjectId.value();
                    }
                    break;

                case DIEFF_OBJECTOFFSETS:
                    for (int i = 0; i < kAssociatedAxes.count; ++i)
                    {
                        const std::optional<DWORD> kMaybeOffset = associatedDevice.IdentifyObjectByOffset({.type = Controller::EElementType::Axis, .axis = kAssociatedAxes.type[i]});
                        if (false == kMaybeOffset.has_value())
                        {
                            // This can happen if the application's data format is not set or if it somehow changed and now does not contain one of the axes that are associated with this effect object.
                            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
                        }

                        peff->rgdwAxes[i] = kMaybeOffset.value();
                    }
                    break;

                default:
                    // It is an error if the caller does not specify exactly one specific way of identifying axis objects.
                    LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
                }
            }
        }

        bool directionInsufficientBuffer = false;
        if (0 != (dwFlags & DIEP_DIRECTION))
        {
            if (false == effect->HasDirection())
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

            const Controller::ForceFeedback::DirectionVector& kDirectionVector = effect->Direction();
            if (peff->cAxes < (DWORD)kDirectionVector.GetNumAxes())
            {
                peff->cAxes = (DWORD)kDirectionVector.GetNumAxes();
                directionInsufficientBuffer = true;
            }
            else
            {
                if (nullptr == peff->rglDirection)
                    LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

                if (0 == (peff->dwFlags & (DIEFF_CARTESIAN | DIEFF_POLAR | DIEFF_SPHERICAL)))
                    LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

                const std::optional<Controller::ForceFeedback::ECoordinateSystem> kMaybeCoordinateSystem = PickCoordinateSystem(kDirectionVector, peff->dwFlags);
                if (false == kMaybeCoordinateSystem.has_value())
                    LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

                Controller::ForceFeedback::TEffectValue coordinates[Controller::ForceFeedback::kEffectAxesMaximumNumber] = {};
                int numCoordinates = 0;

                // The selected coordinate system is identified to the caller by ensuring exactly one coordinate system is saved into the flags on output.
                // First the coordinate system flags are all cleared, then in the switch block below a single system is added back into the flags.
                peff->dwFlags = (peff->dwFlags & ~(DIEFF_CARTESIAN | DIEFF_POLAR | DIEFF_SPHERICAL));

                switch (kMaybeCoordinateSystem.value())
                {
                case Controller::ForceFeedback::ECoordinateSystem::Cartesian:
                    numCoordinates = kDirectionVector.GetCartesianCoordinates(coordinates, _countof(coordinates));
                    peff->dwFlags |= DIEFF_CARTESIAN;
                    break;

                case Controller::ForceFeedback::ECoordinateSystem::Polar:
                    numCoordinates = kDirectionVector.GetPolarCoordinates(coordinates, _countof(coordinates));
                    peff->dwFlags |= DIEFF_POLAR;
                    break;

                case Controller::ForceFeedback::ECoordinateSystem::Spherical:
                    numCoordinates = kDirectionVector.GetSphericalCoordinates(coordinates, _countof(coordinates));
                    peff->dwFlags |= DIEFF_SPHERICAL;
                    break;
                }

                if (0 == numCoordinates)
                {
                    // This should never happen. It means the direction is supposedly present and the coordinate system selected is supposedly valid but coordinate values were unable to be retrieved.
                    Message::OutputFormatted(Message::ESeverity::Error, L"Internal error while retrieving direction components using coordinate system %d on Xidi virtual controller %u.", (int)(kMaybeCoordinateSystem.value()), (1 + associatedDevice.GetVirtualController().GetIdentifier()));
                    LOG_INVOCATION_AND_RETURN(DIERR_GENERIC, kMethodSeverity);
                }

                for (DWORD i = 0; i < (DWORD)numCoordinates; ++i)
                    peff->rglDirection[i] = (LONG)coordinates[i];

                for (DWORD i = (DWORD)numCoordinates; i < peff->cAxes; ++i)
                    peff->rglDirection[i] = 0;
            }
        }

        if (0 != (dwFlags & DIEP_DURATION))
        {
            if (false == effect->HasDuration())
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

            peff->dwDuration = ConvertTimeToDirectInput(effect->GetDuration().value());
        }

        if (0 != (dwFlags & DIEP_ENVELOPE))
        {
            if (false == effect->HasEnvelope())
                peff->lpEnvelope = nullptr;
            else
            {
                if (nullptr == peff->lpEnvelope)
                    LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

                if (sizeof(DIENVELOPE) != peff->lpEnvelope->dwSize)
                    LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

                const Controller::ForceFeedback::SEnvelope kEnvelope = effect->GetEnvelope().value();
                peff->lpEnvelope->dwAttackLevel = (DWORD)kEnvelope.attackLevel;
                peff->lpEnvelope->dwAttackTime = ConvertTimeToDirectInput(kEnvelope.attackTime);
                peff->lpEnvelope->dwFadeLevel = (DWORD)kEnvelope.fadeLevel;
                peff->lpEnvelope->dwFadeTime = ConvertTimeToDirectInput(kEnvelope.fadeTime);
            }
        }

        if (0 != (dwFlags & DIEP_GAIN))
            peff->dwGain = (DWORD)effect->GetGain();

        if (0 != (dwFlags & DIEP_SAMPLEPERIOD))
            peff->dwSamplePeriod = ConvertTimeToDirectInput(effect->GetSamplePeriod());

        bool typeSpecificInsufficientBuffer = false;
        if (0 != (dwFlags & DIEP_TYPESPECIFICPARAMS))
        {
            switch (GetTypeSpecificParameters(peff))
            {
            case DI_OK:
                break;

            case DIERR_MOREDATA:
                typeSpecificInsufficientBuffer = true;
                break;

            default:
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
            }
        }

        const HRESULT kOverallResult = ((true == (axesInsufficientBuffer || directionInsufficientBuffer || typeSpecificInsufficientBuffer)) ? DIERR_MOREDATA : DI_OK);
        LOG_INVOCATION_AND_RETURN(kOverallResult, kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::SetParameters(LPCDIEFFECT peff, DWORD dwFlags)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(SetParametersInternal(peff, dwFlags), kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Start(DWORD dwIterations, DWORD dwFlags)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(StartInternal(dwIterations, dwFlags), kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Stop(void)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        Controller::ForceFeedback::Device* const forceFeedbackDevice = associatedDevice.AutoAcquireAndGetForceFeedbackDevice();
        if (nullptr == forceFeedbackDevice)
            LOG_INVOCATION_AND_RETURN(DIERR_NOTEXCLUSIVEACQUIRED, kMethodSeverity);

        forceFeedbackDevice->StopEffect(effect->Identifier());
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::GetEffectStatus(LPDWORD pdwFlags)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        if (nullptr == pdwFlags)
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        *pdwFlags = 0;

        Controller::ForceFeedback::Device* const forceFeedbackDevice = associatedDevice.GetVirtualController().ForceFeedbackGetDevice();
        if (nullptr == forceFeedbackDevice)
            LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);

        if (true == forceFeedbackDevice->IsEffectPlaying(effect->Identifier()))
            *pdwFlags |= DIEGES_PLAYING;

        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Download(void)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DownloadInternal(), kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Unload(void)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        Controller::ForceFeedback::Device* const forceFeedbackDevice = associatedDevice.AutoAcquireAndGetForceFeedbackDevice();
        if (nullptr == forceFeedbackDevice)
            LOG_INVOCATION_AND_RETURN(DIERR_NOTEXCLUSIVEACQUIRED, kMethodSeverity);
        
        forceFeedbackDevice->RemoveEffect(effect->Identifier());
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Escape(LPDIEFFESCAPE pesc)
    {
        constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }


    // -------- EXPLICIT TEMPLATE INSTANTIATION ---------------------------- //
    // Instantiates both the ASCII and Unicode versions of all needed classes.

    template class VirtualDirectInputEffect<ECharMode::A>;
    template class VirtualDirectInputEffect<ECharMode::W>;
    template class ConstantForceDirectInputEffect<ECharMode::A>;
    template class ConstantForceDirectInputEffect<ECharMode::W>;
    template class PeriodicDirectInputEffect<ECharMode::A>;
    template class PeriodicDirectInputEffect<ECharMode::W>;
    template class RampForceDirectInputEffect<ECharMode::A>;
    template class RampForceDirectInputEffect<ECharMode::W>;
}
