/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file VirtualDirectInputDevice.cpp
 *   Implementation of an IDirectInputDevice interface wrapper around virtual
 *   controllers.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "ControllerIdentification.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "Message.h"
#include "Strings.h"
#include "VirtualController.h"
#include "VirtualDirectInputDevice.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <memory>
#include <optional>


// -------- MACROS --------------------------------------------------------- //

/// Logs a DirectInput interface method invocation and returns.
#define LOG_INVOCATION_AND_RETURN(result, severity) \
    do \
    { \
        const HRESULT kResult = (result); \
        Message::OutputFormatted(severity, L"Invoked %s on Xidi virtual controller %u, result = 0x%08x.", __FUNCTIONW__ L"()", (1 + controller->GetIdentifier()), kResult); \
        return kResult; \
    } while (false)

/// Logs a DirectInput property-related method invocation and returns.
#define LOG_PROPERTY_INVOCATION_AND_RETURN(result, severity, rguidprop, propvalfmt, ...) \
    do \
    { \
        const HRESULT kResult = (result); \
        Message::OutputFormatted(severity, L"Invoked function %s on Xidi virtual controller %u, result = 0x%08x, property = %s" propvalfmt L".", __FUNCTIONW__ L"()", (1 + controller->GetIdentifier()), kResult, PropertyGuidString(rguidprop), ##__VA_ARGS__); \
        return kResult; \
    } while (false)

/// Logs a DirectInput property-related method without a value and returns.
#define LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(result, severity, rguidprop)                LOG_PROPERTY_INVOCATION_AND_RETURN(result, severity, rguidprop, L"")

/// Logs a DirectInput property-related method where the value is provided in a DIPROPDWORD structure and returns.
#define LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(result, severity, rguidprop, ppropval)   LOG_PROPERTY_INVOCATION_AND_RETURN(result, severity, rguidprop, L", value = { dwData = %u }", ((LPDIPROPDWORD)ppropval)->dwData)

/// Logs a DirectInput property-related method where the value is provided in a DIPROPRANGE structure and returns.
#define LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(result, severity, rguidprop, ppropval)   LOG_PROPERTY_INVOCATION_AND_RETURN(result, severity, rguidprop, L", value = { lMin = %ld, lMax = %ld }", ((LPDIPROPRANGE)ppropval)->lMin, ((LPDIPROPRANGE)ppropval)->lMax)



/// Produces and returns a human-readable string from a given DirectInput property GUID.
#define DI_PROPERTY_STRING(rguid, diprop) if (&diprop == &rguid) return _CRT_WIDE(#diprop);


namespace Xidi
{
    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Converts from axis type enumerator to axis type GUID.
    /// @param [in] axis Axis type enumerator to convert.
    /// @return Read-only reference to the corresponding GUID object.
    static const GUID& AxisTypeGuid(Controller::EAxis axis)
    {
        switch (axis)
        {
        case Controller::EAxis::X:
            return GUID_XAxis;
        case Controller::EAxis::Y:
            return GUID_YAxis;
        case Controller::EAxis::Z:
            return GUID_ZAxis;
        case Controller::EAxis::RotX:
            return GUID_RxAxis;
        case Controller::EAxis::RotY:
            return GUID_RyAxis;
        case Controller::EAxis::RotZ:
            return GUID_RzAxis;
        default:
            return GUID_Unknown;
        }
    }

    /// Returns a string representation of the way in which a controller element is identified.
    /// @param [in] dwHow Value to check.
    /// @return String representation of the identification method.
    static const wchar_t* IdentificationMethodString(DWORD dwHow)
    {
        switch (dwHow)
        {
        case DIPH_DEVICE:
            return L"DIPH_DEVICE";
        case DIPH_BYOFFSET:
            return L"DIPH_BYOFFSET";
        case DIPH_BYUSAGE:
            return L"DIPH_BYUSAGE";
        case DIPH_BYID:
            return L"DIPH_BYID";
        default:
            return L"(unknown)";
        }
    }

    /// Returns a human-readable string that represents the specified property GUID.
    /// @param [in] pguid GUID to check.
    /// @return String representation of the GUID's semantics.
    static const wchar_t* PropertyGuidString(REFGUID rguidProp)
    {
        switch ((size_t)&rguidProp)
        {
#if DIRECTINPUT_VERSION >= 0x0800
        case ((size_t)&DIPROP_KEYNAME):
            return L"DIPROP_KEYNAME";
        case ((size_t)&DIPROP_CPOINTS):
            return L"DIPROP_CPOINTS";
        case ((size_t)&DIPROP_APPDATA):
            return L"DIPROP_APPDATA";
        case ((size_t)&DIPROP_SCANCODE):
            return L"DIPROP_SCANCODE";
        case ((size_t)&DIPROP_VIDPID):
            return L"DIPROP_VIDPID";
        case ((size_t)&DIPROP_USERNAME):
            return L"DIPROP_USERNAME";
        case ((size_t)&DIPROP_TYPENAME):
            return L"DIPROP_TYPENAME";
#endif
        case ((size_t)&DIPROP_BUFFERSIZE):
            return L"DIPROP_BUFFERSIZE";
        case ((size_t)&DIPROP_AXISMODE):
            return L"DIPROP_AXISMODE";
        case ((size_t)&DIPROP_GRANULARITY):
            return L"DIPROP_GRANULARITY";
        case ((size_t)&DIPROP_RANGE):
            return L"DIPROP_RANGE";
        case ((size_t)&DIPROP_DEADZONE):
            return L"DIPROP_DEADZONE";
        case ((size_t)&DIPROP_SATURATION):
            return L"DIPROP_SATURATION";
        case ((size_t)&DIPROP_FFGAIN):
            return L"DIPROP_FFGAIN";
        case ((size_t)&DIPROP_FFLOAD):
            return L"DIPROP_FFLOAD";
        case ((size_t)&DIPROP_AUTOCENTER):
            return L"DIPROP_AUTOCENTER";
        case ((size_t)&DIPROP_CALIBRATIONMODE):
            return L"DIPROP_CALIBRATIONMODE";
        case ((size_t)&DIPROP_CALIBRATION):
            return L"DIPROP_CALIBRATION";
        case ((size_t)&DIPROP_GUIDANDPATH):
            return L"DIPROP_GUIDANDPATH";
        case ((size_t)&DIPROP_INSTANCENAME):
            return L"DIPROP_INSTANCENAME";
        case ((size_t)&DIPROP_PRODUCTNAME):
            return L"DIPROP_PRODUCTNAME";
        case ((size_t)&DIPROP_JOYSTICKID):
            return L"DIPROP_JOYSTICKID";
        case ((size_t)&DIPROP_GETPORTDISPLAYNAME):
            return L"DIPROP_GETPORTDISPLAYNAME";
        case ((size_t)&DIPROP_PHYSICALRANGE):
            return L"DIPROP_PHYSICALRANGE";
        case ((size_t)&DIPROP_LOGICALRANGE):
            return L"DIPROP_LOGICALRANGE";
        default:
            return L"(unknown)";
        }
    }

    /// Performs property-specific validation of the supplied property header.
    /// Ensures the header exists and all sizes are correct.
    /// @param [in] rguidProp GUID of the property for which the header is being validated.
    /// @param [in] pdiph Pointer to the property header structure to validate.
    /// @return `true` if the header is valid, `false` otherwise.
    static bool IsPropertyHeaderValid(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
    {
        if (nullptr == pdiph)
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"Rejected null property header for %s.", PropertyGuidString(rguidProp));
            return false;
        }
        else if ((sizeof(DIPROPHEADER) != pdiph->dwHeaderSize))
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"Rejected invalid property header for %s: Incorrect size for DIPROPHEADER (expected %u, got %u).", PropertyGuidString(rguidProp), (unsigned int)sizeof(DIPROPHEADER), (unsigned int)pdiph->dwHeaderSize);
            return false;
        }
        else if ((DIPH_DEVICE == pdiph->dwHow) && (0 != pdiph->dwObj))
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"Rejected invalid property header for %s: Incorrect object identification value used with DIPH_DEVICE (expected %u, got %u).", PropertyGuidString(rguidProp), (unsigned int)0, (unsigned int)pdiph->dwObj);
            return false;
        }

        // Look for reasons why the property header might be invalid and reject it if any are found.
        switch ((size_t)&rguidProp)
        {
        case ((size_t)&DIPROP_AXISMODE):
        case ((size_t)&DIPROP_DEADZONE):
        case ((size_t)&DIPROP_SATURATION):
            // Axis mode, deadzone, and saturation all use DIPROPDWORD.
            if (sizeof(DIPROPDWORD) != pdiph->dwSize)
            {
                Message::OutputFormatted(Message::ESeverity::Warning, L"Rejected invalid property header for %s: Incorrect size for DIPROPDWORD (expected %u, got %u).", PropertyGuidString(rguidProp), (unsigned int)sizeof(DIPROPDWORD), (unsigned int)pdiph->dwSize);
                return false;
            }
            break;

        case ((size_t)&DIPROP_BUFFERSIZE):
            // Buffer size uses DIPROPDWORD and is exclusively a device-wide property.
            if (DIPH_DEVICE != pdiph->dwHow)
            {
                Message::OutputFormatted(Message::ESeverity::Warning, L"Rejected invalid property header for %s: Incorrect object identification method for this property (expected %s, got %s).", PropertyGuidString(rguidProp), IdentificationMethodString(DIPH_DEVICE), IdentificationMethodString(pdiph->dwHow));
                return false;
            }
            else if (sizeof(DIPROPDWORD) != pdiph->dwSize)
            {
                Message::OutputFormatted(Message::ESeverity::Warning, L"Rejected invalid property header for %s: Incorrect size for DIPROPDWORD (expected %u, got %u).", PropertyGuidString(rguidProp), (unsigned int)sizeof(DIPROPDWORD), (unsigned int)pdiph->dwSize);
                return false;
            }
            break;

        case ((size_t)&DIPROP_RANGE):
            // Range uses DIPROPRANGE.
            if (sizeof(DIPROPRANGE) != pdiph->dwSize)
            {
                Message::OutputFormatted(Message::ESeverity::Warning, L"Rejected invalid property header for %s: Incorrect size for DIPROPRANGE (expected %u, got %u).", PropertyGuidString(rguidProp), (unsigned int)sizeof(DIPROPRANGE), (unsigned int)pdiph->dwSize);
                return false;
            }
            break;

        default:
            // Any property not listed here is not supported by Xidi and therefore not validated by it.
            Message::OutputFormatted(Message::ESeverity::Warning, L"Skipped property header validation because the property %s is not supported.", PropertyGuidString(rguidProp));
            return true;
        }

        Message::OutputFormatted(Message::ESeverity::Info, L"Accepted valid property header for %s.", PropertyGuidString(rguidProp));
        return true;
    }
    
    /// Dumps the top-level components of a property request.
    /// @param [in] rguidProp GUID of the property.
    /// @param [in] pdiph Pointer to the property header.
    /// @param [in] requestTypeIsSet `true` if the request type is SetProperty, `false` if it is GetProperty.
    static void DumpPropertyRequest(REFGUID rguidProp, LPCDIPROPHEADER pdiph, bool requestTypeIsSet)
    {
        constexpr Message::ESeverity kDumpSeverity = Message::ESeverity::Debug;

        if (Message::WillOutputMessageOfSeverity(kDumpSeverity))
        {
            Message::Output(kDumpSeverity, L"Begin dump of property request.");

            Message::Output(kDumpSeverity, L"  Metadata:");
            Message::OutputFormatted(kDumpSeverity, L"    operation = %sProperty", ((true == requestTypeIsSet) ? L"Set" : L"Get"));
            Message::OutputFormatted(kDumpSeverity, L"    rguidProp = %s", PropertyGuidString(rguidProp));

            Message::Output(kDumpSeverity, L"  Header:");
            if (nullptr == pdiph)
            {
                Message::Output(kDumpSeverity, L"    (missing)");
            }
            else
            {
                Message::OutputFormatted(kDumpSeverity, L"    dwSize = %u", pdiph->dwSize);
                Message::OutputFormatted(kDumpSeverity, L"    dwHeaderSize = %u", pdiph->dwHeaderSize);
                Message::OutputFormatted(kDumpSeverity, L"    dwObj = %u (0x%08x)", pdiph->dwObj, pdiph->dwObj);
                Message::OutputFormatted(kDumpSeverity, L"    dwHow = %u (%s)", pdiph->dwHow, IdentificationMethodString(pdiph->dwHow));
            }

            Message::Output(kDumpSeverity, L"End dump of property request.");
        }
    }
    
    /// Fills the specified buffer with a friendly string representation of the specified controller element.
    /// Default version does nothing.
    /// @tparam CharType String character type, either `char` or `wchar_t`.
    /// @param [in] element Controller element for which a string is desired.
    /// @param [out] buf Buffer to be filled with the string.
    /// @param [in] bufcount Buffer size in number of characters.
    template <typename CharType> static void ElementToString(Controller::SElementIdentifier element, CharType* buf, int bufcount)
    {
        // Nothing to do here.
    }

    /// Fills the specified buffer with a friendly string representation of the specified controller element, specialized for ASCII.
    /// @param [in] element Controller element for which a string is desired.
    /// @param [out] buf Buffer to be filled with the string.
    /// @param [in] bufcount Buffer size in number of characters.
    template <> static void ElementToString<char>(Controller::SElementIdentifier element, char* buf, int bufcount)
    {
        switch (element.type)
        {
        case Controller::EElementType::Axis:
            switch (element.axis)
            {
            case Controller::EAxis::X:
                strncpy_s(buf, bufcount, XIDI_AXIS_NAME_X, _countof(XIDI_AXIS_NAME_X));
                break;
            case Controller::EAxis::Y:
                strncpy_s(buf, bufcount, XIDI_AXIS_NAME_Y, _countof(XIDI_AXIS_NAME_Y));
                break;
            case Controller::EAxis::Z:
                strncpy_s(buf, bufcount, XIDI_AXIS_NAME_Z, _countof(XIDI_AXIS_NAME_Z));
                break;
            case Controller::EAxis::RotX:
                strncpy_s(buf, bufcount, XIDI_AXIS_NAME_RX, _countof(XIDI_AXIS_NAME_RX));
                break;
            case Controller::EAxis::RotY:
                strncpy_s(buf, bufcount, XIDI_AXIS_NAME_RY, _countof(XIDI_AXIS_NAME_RY));
                break;
            case Controller::EAxis::RotZ:
                strncpy_s(buf, bufcount, XIDI_AXIS_NAME_RZ, _countof(XIDI_AXIS_NAME_RZ));
                break;
            default:
                strncpy_s(buf, bufcount, XIDI_AXIS_NAME_UNKNOWN, _countof(XIDI_AXIS_NAME_UNKNOWN));
                break;
            }
            break;

        case Controller::EElementType::Button:
            sprintf_s(buf, bufcount, XIDI_BUTTON_NAME_FORMAT, (1 + (unsigned int)element.button));
            break;

        case Controller::EElementType::Pov:
            strncpy_s(buf, bufcount, XIDI_POV_NAME, _countof(XIDI_POV_NAME));
            break;

        case Controller::EElementType::WholeController:
            strncpy_s(buf, bufcount, XIDI_WHOLE_CONTROLLER_NAME, _countof(XIDI_WHOLE_CONTROLLER_NAME));
            break;
        }
    }

    /// Fills the specified buffer with a friendly string representation of the specified controller element, specialized for ASCII.
    /// @param [in] element Controller element for which a string is desired.
    /// @param [out] buf Buffer to be filled with the string.
    /// @param [in] bufcount Buffer size in number of characters.
    template <> static void ElementToString<wchar_t>(Controller::SElementIdentifier element, wchar_t* buf, int bufcount)
    {
        switch (element.type)
        {
        case Controller::EElementType::Axis:
            switch (element.axis)
            {
            case Controller::EAxis::X:
                wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_X), _countof(_CRT_WIDE(XIDI_AXIS_NAME_X)));
                break;
            case Controller::EAxis::Y:
                wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_Y), _countof(_CRT_WIDE(XIDI_AXIS_NAME_Y)));
                break;
            case Controller::EAxis::Z:
                wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_Z), _countof(_CRT_WIDE(XIDI_AXIS_NAME_Z)));
                break;
            case Controller::EAxis::RotX:
                wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_RX), _countof(_CRT_WIDE(XIDI_AXIS_NAME_RX)));
                break;
            case Controller::EAxis::RotY:
                wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_RY), _countof(_CRT_WIDE(XIDI_AXIS_NAME_RY)));
                break;
            case Controller::EAxis::RotZ:
                wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_RZ), _countof(_CRT_WIDE(XIDI_AXIS_NAME_RZ)));
                break;
            default:
                wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_UNKNOWN), _countof(_CRT_WIDE(XIDI_AXIS_NAME_UNKNOWN)));
                break;
            }
            break;

        case Controller::EElementType::Button:
            swprintf_s(buf, bufcount, _CRT_WIDE(XIDI_BUTTON_NAME_FORMAT), (1 + (unsigned int)element.button));
            break;

        case Controller::EElementType::Pov:
            wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_POV_NAME), _countof(_CRT_WIDE(XIDI_POV_NAME)));
            break;

        case Controller::EElementType::WholeController:
            wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_WHOLE_CONTROLLER_NAME), _countof(_CRT_WIDE(XIDI_WHOLE_CONTROLLER_NAME)));
            break;
        }
    }

    /// Computes the offset in a virtual controller's "native" data packet.
    /// For application information only. Cannot be used to identify objects.
    /// Application is presented with the image of a native data packet that stores axes first, then buttons (one byte per button), then POV.
    /// @param [in] controllerElement Virtual controller element for which a native offset is desired.
    /// @return Native offset value for providing to applications.
    static TOffset NativeOffsetForElement(Controller::SElementIdentifier controllerElement)
    {
        switch (controllerElement.type)
        {
        case Controller::EElementType::Axis:
            return offsetof(Controller::SState, axis[(int)controllerElement.axis]);

        case Controller::EElementType::Button:
            return offsetof(Controller::SState, button) + (TOffset)controllerElement.button;

        case Controller::EElementType::Pov:
            return offsetof(Controller::SState, button) + (TOffset)Controller::EButton::Count;

        default:                                                                                    // This should never happen.
            return DataFormat::kInvalidOffsetValue;
        }
    }

    /// Fills the specified object instance information structure with information about the specified controller element.
    /// @param [in] controllerCapabilities Capabilities that describe the layout of the virtual controller.
    /// @param [in] controllerElement Virtual controller element about which to fill information.
    /// @param [in] offset Offset to place into the object instance information structure.
    /// @param objectInfo [out] Structure to be filled with instance information.
    template <ECharMode charMode> static void FillObjectInstanceInfo(const Controller::SCapabilities controllerCapabilities, Controller::SElementIdentifier controllerElement, TOffset offset, typename DirectInputDeviceType<charMode>::DeviceObjectInstanceType* objectInfo)
    {
        ZeroMemory(objectInfo, sizeof(*objectInfo));
        objectInfo->dwSize = sizeof(*objectInfo);
        objectInfo->dwOfs = offset;
        ElementToString(controllerElement, objectInfo->tszName, _countof(objectInfo->tszName));

        switch (controllerElement.type)
        {
        case Controller::EElementType::Axis:
            objectInfo->guidType = AxisTypeGuid(controllerElement.axis);
            objectInfo->dwType = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE((int)controllerCapabilities.FindAxis(controllerElement.axis));
            objectInfo->dwFlags = DIDOI_POLLED | DIDOI_ASPECTPOSITION;
            break;

        case Controller::EElementType::Button:
            objectInfo->guidType = GUID_Button;
            objectInfo->dwType = DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE((int)controllerElement.button);
            objectInfo->dwFlags = DIDOI_POLLED;
            break;

        case Controller::EElementType::Pov:
            objectInfo->guidType = GUID_POV;
            objectInfo->dwType = DIDFT_POV | DIDFT_MAKEINSTANCE(0);
            objectInfo->dwFlags = DIDOI_POLLED;
            break;
        }
    }

    /// Signals the specified event if its handle is valid.
    /// @param [in] eventHandle Handle that can be used to identify the desired event object.
    static inline void SignalEventIfEnabled(HANDLE eventHandle)
    {
        if ((NULL != eventHandle) && (INVALID_HANDLE_VALUE != eventHandle))
            SetEvent(eventHandle);
    }


    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "VirtualDirectInputDevice.h" for documentation.

    template <ECharMode charMode> VirtualDirectInputDevice<charMode>::VirtualDirectInputDevice(std::unique_ptr<Controller::VirtualController>&& controller) : controller(std::move(controller)), dataFormat(), refCount(1), stateChangeEventHandle(NULL)
    {
        // Nothing to do here.
    }


    // -------- INSTANCE METHODS ------------------------------------------- //
    // See "VirtualDirectInputDevice.h" for documentation.

    template <ECharMode charMode> std::optional<Controller::SElementIdentifier> VirtualDirectInputDevice<charMode>::IdentifyElement(DWORD dwObj, DWORD dwHow) const
    {
        switch (dwHow)
        {
        case DIPH_DEVICE:
            // Whole device is referenced.
            // Per DirectInput documentation, the object identifier must be 0.
            if (0 == dwObj)
                return Controller::SElementIdentifier({.type = Controller::EElementType::WholeController});
            break;

        case DIPH_BYOFFSET:
            // Controller element is being identified by offset.
            // Object identifier is an offset into the application's data format.
            if (true == IsApplicationDataFormatSet())
                return dataFormat->GetElementForOffset(dwObj);
            break;

        case DIPH_BYID:
            // Controller element is being identified by instance identifier.
            // Object identifier contains type and index, and the latter refers to the controller's reported capabilities.
            if ((unsigned int)DIDFT_GETINSTANCE(dwObj) >= 0)
            {
                const unsigned int kType = (unsigned int)DIDFT_GETTYPE(dwObj);
                const unsigned int kIndex = (unsigned int)DIDFT_GETINSTANCE(dwObj);

                switch (kType)
                {
                case DIDFT_ABSAXIS:
                    if ((kIndex < (unsigned int)Controller::EAxis::Count) && (kIndex < (unsigned int)controller->GetCapabilities().numAxes))
                        return Controller::SElementIdentifier({.type = Controller::EElementType::Axis, .axis = controller->GetCapabilities().axisType[kIndex]});
                    break;

                case DIDFT_PSHBUTTON:
                    if ((kIndex < (unsigned int)Controller::EButton::Count) && (kIndex < (unsigned int)controller->GetCapabilities().numButtons))
                        return Controller::SElementIdentifier({.type = Controller::EElementType::Button, .button = (Controller::EButton)kIndex});
                    break;

                case DIDFT_POV:
                    if (kIndex == 0)
                        return Controller::SElementIdentifier({.type = Controller::EElementType::Pov});
                    break;
                }
            }
            break;
        }

        return std::nullopt;
    }


    // -------- METHODS: IUnknown ------------------------------------------ //
    // See IUnknown documentation for more information.

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::QueryInterface(REFIID riid, LPVOID* ppvObj)
    {
        if (nullptr == ppvObj)
            return E_POINTER;

        bool validInterfaceRequested = false;

        if (ECharMode::W == charMode)
        {
#if DIRECTINPUT_VERSION >= 0x0800
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice8W))
#else
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice7W) || IsEqualIID(riid, IID_IDirectInputDevice2W) || IsEqualIID(riid, IID_IDirectInputDeviceW))
#endif
                validInterfaceRequested = true;
        }
        else
        {
#if DIRECTINPUT_VERSION >= 0x0800
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice8A))
#else
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice7A) || IsEqualIID(riid, IID_IDirectInputDevice2A) || IsEqualIID(riid, IID_IDirectInputDeviceA))
#endif
                validInterfaceRequested = true;
        }

        if (true == validInterfaceRequested)
        {
            AddRef();
            *ppvObj = this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    // ---------

    template <ECharMode charMode> ULONG VirtualDirectInputDevice<charMode>::AddRef(void)
    {
        return ++refCount;
    }

    // ---------

    template <ECharMode charMode> ULONG VirtualDirectInputDevice<charMode>::Release(void)
    {
        const unsigned long numRemainingRefs = --refCount;

        if (0 == numRemainingRefs)
            delete this;

        return (ULONG)numRemainingRefs;
    }


    // -------- METHODS: IDirectInputDevice COMMON ------------------------- //
    // See DirectInput documentation for more information.

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::Acquire(void)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        
        // Controller acquisition is a no-op for Xidi virtual controllers.
        // However, DirectInput documentation requires that the application data format already be set.
        if (false == IsApplicationDataFormatSet())
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::EnumEffects(DirectInputDeviceType<charMode>::EnumEffectsCallbackType lpCallback, LPVOID pvRef, DWORD dwEffType)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::EnumEffectsInFile(DirectInputDeviceType<charMode>::ConstStringType lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::EnumObjects(DirectInputDeviceType<charMode>::EnumObjectsCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        const bool willEnumerateAxes = ((DIDFT_ALL == dwFlags) || (0 != (dwFlags & DIDFT_ABSAXIS)));
        const bool willEnumerateButtons = ((DIDFT_ALL == dwFlags) || (0 != (dwFlags & DIDFT_PSHBUTTON)));
        const bool willEnumeratePov = ((DIDFT_ALL == dwFlags) || (0 != (dwFlags & DIDFT_POV)));

        if (nullptr == lpCallback)
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        if ((true == willEnumerateAxes) || (true == willEnumerateButtons) || (true == willEnumeratePov))
        {
            std::unique_ptr<DirectInputDeviceType<charMode>::DeviceObjectInstanceType> objectDescriptor = std::make_unique<DirectInputDeviceType<charMode>::DeviceObjectInstanceType>();
            const Controller::SCapabilities controllerCapabilities = controller->GetCapabilities();

            if (true == willEnumerateAxes)
            {
                for (int i = 0; i < controllerCapabilities.numAxes; ++i)
                {
                    const Controller::EAxis kAxis = controllerCapabilities.axisType[i];
                    const Controller::SElementIdentifier kAxisIdentifier = {.type = Controller::EElementType::Axis, .axis = kAxis};
                    const TOffset kAxisOffset = ((true == IsApplicationDataFormatSet()) ? dataFormat->GetOffsetForElement(kAxisIdentifier).value_or(DataFormat::kInvalidOffsetValue) : NativeOffsetForElement(kAxisIdentifier));

                    FillObjectInstanceInfo<charMode>(controllerCapabilities, kAxisIdentifier, kAxisOffset, objectDescriptor.get());
                    switch (lpCallback(objectDescriptor.get(), pvRef))
                    {
                    case DIENUM_CONTINUE:
                        break;
                    case DIENUM_STOP:
                        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
                    default:
                        LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
                    }
                }
            }

            if (true == willEnumerateButtons)
            {
                for (int i = 0; i < controllerCapabilities.numButtons; ++i)
                {
                    const Controller::EButton kButton = (Controller::EButton)i;
                    const Controller::SElementIdentifier kButtonIdentifier = {.type = Controller::EElementType::Button, .button = kButton};
                    const TOffset kButtonOffset = ((true == IsApplicationDataFormatSet()) ? dataFormat->GetOffsetForElement(kButtonIdentifier).value_or(DataFormat::kInvalidOffsetValue) : NativeOffsetForElement(kButtonIdentifier));

                    FillObjectInstanceInfo<charMode>(controllerCapabilities, kButtonIdentifier, kButtonOffset, objectDescriptor.get());
                    switch (lpCallback(objectDescriptor.get(), pvRef))
                    {
                    case DIENUM_CONTINUE:
                        break;
                    case DIENUM_STOP:
                        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
                    default:
                        LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
                    }
                }
            }

            if (true == willEnumeratePov)
            {
                if (true == controllerCapabilities.hasPov)
                {
                    const Controller::SElementIdentifier kPovIdentifier = {.type = Controller::EElementType::Pov};
                    const TOffset kPovOffset = ((true == IsApplicationDataFormatSet()) ? dataFormat->GetOffsetForElement(kPovIdentifier).value_or(DataFormat::kInvalidOffsetValue) : NativeOffsetForElement(kPovIdentifier));
                    
                    FillObjectInstanceInfo<charMode>(controllerCapabilities, kPovIdentifier, kPovOffset, objectDescriptor.get());
                    switch (lpCallback(objectDescriptor.get(), pvRef))
                    {
                    case DIENUM_CONTINUE:
                        break;
                    case DIENUM_STOP:
                        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
                    default:
                        LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
                    }
                }
            }
        }

        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::Escape(LPDIEFFESCAPE pesc)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        if (nullptr == lpDIDevCaps)
            LOG_INVOCATION_AND_RETURN(E_POINTER, kMethodSeverity);
        
        switch (lpDIDevCaps->dwSize)
        {
            case (sizeof(DIDEVCAPS)):
                // Force feedback information, only present in the latest version of the structure.
                lpDIDevCaps->dwFFSamplePeriod = 0;
                lpDIDevCaps->dwFFMinTimeResolution = 0;
                lpDIDevCaps->dwFirmwareRevision = 0;
                lpDIDevCaps->dwHardwareRevision = 0;
                lpDIDevCaps->dwFFDriverVersion = 0;

            case (sizeof(DIDEVCAPS_DX3)):
                // Top-level controller information is common to all virtual controllers.
                lpDIDevCaps->dwFlags = DIDC_ATTACHED | DIDC_EMULATED | DIDC_POLLEDDEVICE | DIDC_POLLEDDATAFORMAT;
                lpDIDevCaps->dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD;
                
                // Information about controller layout comes from controller capabilities.
                lpDIDevCaps->dwAxes = controller->GetCapabilities().numAxes;
                lpDIDevCaps->dwButtons = controller->GetCapabilities().numButtons;
                lpDIDevCaps->dwPOVs = ((true == controller->GetCapabilities().hasPov) ? 1 : 0);
                break;

            default:
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
        }

        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::SuperDebug;
        static constexpr Message::ESeverity kMethodSeverityForError = Message::ESeverity::Info;
        
        if ((false == IsApplicationDataFormatSet()) || (nullptr == pdwInOut) || (sizeof(DIDEVICEOBJECTDATA) != cbObjectData))
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverityForError);

        switch (dwFlags)
        {
        case 0:
        case DIGDD_PEEK:
            break;

        default:
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverityForError);
        }

        if (false == controller->IsEventBufferEnabled())
            LOG_INVOCATION_AND_RETURN(DIERR_NOTBUFFERED, kMethodSeverityForError);

        auto lock = controller->Lock();
        const DWORD kNumEventsAffected = std::min(*pdwInOut, (DWORD)controller->GetEventBufferCount());
        const bool kEventBufferOverflowed = controller->IsEventBufferOverflowed();
        const bool kShouldPopEvents = (0 == (dwFlags & DIGDD_PEEK));

        if (nullptr != rgdod)
        {
            for (DWORD i = 0; i < kNumEventsAffected; ++i)
            {
                const Controller::StateChangeEventBuffer::SEvent& event = controller->GetEventBufferEvent(i);
                ZeroMemory(&rgdod[i], sizeof(rgdod[i]));
                rgdod[i].dwOfs = dataFormat->GetOffsetForElement(event.data.element).value();       // A value should always be present.
                rgdod[i].dwTimeStamp = event.timestamp;
                rgdod[i].dwSequence = event.sequence;

                switch (event.data.element.type)
                {
                case Controller::EElementType::Axis:
                    rgdod[i].dwData = (DWORD)DataFormat::DirectInputAxisValue(event.data.value.axis);
                    break;

                case Controller::EElementType::Button:
                    rgdod[i].dwData = (DWORD)DataFormat::DirectInputButtonValue(event.data.value.button);
                    break;

                case Controller::EElementType::Pov:
                    rgdod[i].dwData = (DWORD)DataFormat::DirectInputPovValue(event.data.value.povDirection);
                    break;

                default:
                    LOG_INVOCATION_AND_RETURN(DIERR_GENERIC, kMethodSeverityForError);              // This should never happen.
                    break;
                }
            }
        }

        if (true == kShouldPopEvents)
            controller->PopEventBufferOldestEvents(kNumEventsAffected);

        *pdwInOut = kNumEventsAffected;
        LOG_INVOCATION_AND_RETURN(((true == kEventBufferOverflowed) ? DI_BUFFEROVERFLOW : DI_OK), kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetDeviceInfo(DirectInputDeviceType<charMode>::DeviceInstanceType* pdidi)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        if (nullptr == pdidi)
            LOG_INVOCATION_AND_RETURN(E_POINTER, kMethodSeverity);
        else if (sizeof(*pdidi) != pdidi->dwSize)
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        FillVirtualControllerInfo(*pdidi, controller->GetIdentifier());
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetDeviceState(DWORD cbData, LPVOID lpvData)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::SuperDebug;
        static constexpr Message::ESeverity kMethodSeverityForError = Message::ESeverity::Info;
        
        if (false == IsApplicationDataFormatSet())
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverityForError);

        bool writeDataPacketResult = false;
        do
        {
            auto lock = controller->Lock();
            writeDataPacketResult = dataFormat->WriteDataPacket(lpvData, cbData, controller->GetStateRef());
        } while (false);
        LOG_INVOCATION_AND_RETURN(((true == writeDataPacketResult) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetEffectInfo(DirectInputDeviceType<charMode>::EffectInfoType* pdei, REFGUID rguid)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetForceFeedbackState(LPDWORD pdwOut)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetObjectInfo(DirectInputDeviceType<charMode>::DeviceObjectInstanceType* pdidoi, DWORD dwObj, DWORD dwHow)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        
        if (nullptr == pdidoi)
            LOG_INVOCATION_AND_RETURN(E_POINTER, kMethodSeverity);
        else if (sizeof(*pdidoi) != pdidoi->dwSize)
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        const std::optional<Controller::SElementIdentifier> maybeElement = IdentifyElement(dwObj, dwHow);
        if (false == maybeElement.has_value())
            LOG_INVOCATION_AND_RETURN(DIERR_OBJECTNOTFOUND, kMethodSeverity);

        const Controller::SElementIdentifier element = maybeElement.value();
        FillObjectInstanceInfo<charMode>(controller->GetCapabilities(), element, ((true == IsApplicationDataFormatSet()) ? dataFormat->GetOffsetForElement(element).value_or(DataFormat::kInvalidOffsetValue) : NativeOffsetForElement(element)), pdidoi);
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        DumpPropertyRequest(rguidProp, pdiph, false);

        if (false == IsPropertyHeaderValid(rguidProp, pdiph))
            LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);

        const std::optional<Controller::SElementIdentifier> maybeElement = IdentifyElement(pdiph->dwObj, pdiph->dwHow);
        if (false == maybeElement.has_value())
            LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_OBJECTNOTFOUND, kMethodSeverity, rguidProp);

        const Controller::SElementIdentifier element = maybeElement.value();

        switch ((size_t)&rguidProp)
        {
        case ((size_t)&DIPROP_AXISMODE):
            ((LPDIPROPDWORD)pdiph)->dwData = DIPROPAXISMODE_ABS;
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

        case ((size_t)&DIPROP_BUFFERSIZE):
            ((LPDIPROPDWORD)pdiph)->dwData = controller->GetEventBufferCapacity();
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

        case ((size_t)&DIPROP_DEADZONE):
            if (Controller::EElementType::Axis != element.type)
                LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);
            ((LPDIPROPDWORD)pdiph)->dwData = controller->GetAxisDeadzone(element.axis);
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

        case ((size_t)&DIPROP_RANGE):
            do
            {
                if (Controller::EElementType::Axis != element.type)
                    LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);

                const std::pair kRange = controller->GetAxisRange(element.axis);
                ((LPDIPROPRANGE)pdiph)->lMin = kRange.first;
                ((LPDIPROPRANGE)pdiph)->lMax = kRange.second;
            } while (false);
            LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);
        
        case ((size_t)&DIPROP_SATURATION):
            if (Controller::EElementType::Axis != element.type)
                LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);
            ((LPDIPROPDWORD)pdiph)->dwData = controller->GetAxisSaturation(element.axis);
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

        default:
            LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity, rguidProp);
        }
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
    {
        // Not required for Xidi virtual controllers as they are implemented now.
        // However, this method is needed for creating IDirectInputDevice objects via COM.

        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::Poll(void)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::SuperDebug;

        // DirectInput documentation requires that the application data format already be set before a device can be polled.
        if (false == IsApplicationDataFormatSet())
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        if (true == controller->RefreshState())
            SignalEventIfEnabled(stateChangeEventHandle);

        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::SendForceFeedbackCommand(DWORD dwFlags)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
    {
        // Presently this is a no-op.
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::SetDataFormat(LPCDIDATAFORMAT lpdf)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        if (nullptr == lpdf)
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
        
        // If this operation fails, then the current data format and event filter remain unaltered.
        std::unique_ptr<DataFormat> newDataFormat = DataFormat::CreateFromApplicationFormatSpec(*lpdf, controller->GetCapabilities());
        if (nullptr == newDataFormat)
            LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

        // Use the event filter to prevent the controller from buffering any events that correspond to elements with no offsets.
        auto lock = controller->Lock();
        controller->EventFilterAddAllElements();
        
        for (int i = 0; i < (int)Controller::EAxis::Count; ++i)
        {
            const Controller::SElementIdentifier kElement = {.type = Controller::EElementType::Axis, .axis = (Controller::EAxis)i};
            if (false == newDataFormat->HasElement(kElement))
                controller->EventFilterRemoveElement(kElement);
        }

        for (int i = 0; i < (int)Controller::EButton::Count; ++i)
        {
            const Controller::SElementIdentifier kElement = {.type = Controller::EElementType::Button, .button = (Controller::EButton)i};
            if (false == newDataFormat->HasElement(kElement))
                controller->EventFilterRemoveElement(kElement);
        }

        do
        {
            const Controller::SElementIdentifier kElement = {.type = Controller::EElementType::Pov};
            if (false == newDataFormat->HasElement(kElement))
                controller->EventFilterRemoveElement(kElement);
        } while (false);

        dataFormat = std::move(newDataFormat);
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::SetEventNotification(HANDLE hEvent)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        stateChangeEventHandle = hEvent;
        LOG_INVOCATION_AND_RETURN(DI_POLLEDDEVICE, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;

        DumpPropertyRequest(rguidProp, pdiph, true);
        
        if (false == IsPropertyHeaderValid(rguidProp, pdiph))
            LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);

        const std::optional<Controller::SElementIdentifier> maybeElement = IdentifyElement(pdiph->dwObj, pdiph->dwHow);
        if (false == maybeElement.has_value())
            LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_OBJECTNOTFOUND, kMethodSeverity, rguidProp);

        const Controller::SElementIdentifier element = maybeElement.value();

        switch ((size_t)&rguidProp)
        {
        case ((size_t)&DIPROP_AXISMODE):
            if (DIPROPAXISMODE_ABS == ((LPDIPROPDWORD)pdiph)->dwData)
                LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_PROPNOEFFECT, kMethodSeverity, rguidProp, pdiph);
            else
                LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity, rguidProp, pdiph);

        case ((size_t)&DIPROP_BUFFERSIZE):
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(((true == controller->SetEventBufferCapacity(((LPDIPROPDWORD)pdiph)->dwData)) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity, rguidProp, pdiph);

        case ((size_t)&DIPROP_DEADZONE):
            switch (element.type)
            {
            case Controller::EElementType::Axis:
                LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(((true == controller->SetAxisDeadzone(element.axis, ((LPDIPROPDWORD)pdiph)->dwData)) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity, rguidProp, pdiph);
            case Controller::EElementType::WholeController:
                LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(((true == controller->SetAllAxisDeadzone(((LPDIPROPDWORD)pdiph)->dwData)) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity, rguidProp, pdiph);
            default:
                LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
            }

        case ((size_t)&DIPROP_RANGE):
            switch (element.type)
            {
            case Controller::EElementType::Axis:
                LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(((true == controller->SetAxisRange(element.axis, ((LPDIPROPRANGE)pdiph)->lMin, ((LPDIPROPRANGE)pdiph)->lMax)) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity, rguidProp, pdiph);
            case Controller::EElementType::WholeController:
                LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(((true == controller->SetAllAxisRange(((LPDIPROPRANGE)pdiph)->lMin, ((LPDIPROPRANGE)pdiph)->lMax)) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity, rguidProp, pdiph);
            default:
                LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
            }

        case ((size_t)&DIPROP_SATURATION):
            switch (element.type)
            {
            case Controller::EElementType::Axis:
                LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(((true == controller->SetAxisSaturation(element.axis, ((LPDIPROPDWORD)pdiph)->dwData)) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity, rguidProp, pdiph);
            case Controller::EElementType::WholeController:
                LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(((true == controller->SetAllAxisSaturation(((LPDIPROPDWORD)pdiph)->dwData)) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity, rguidProp, pdiph);
            default:
                LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
            }

        default:
            LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity, rguidProp);
        }
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::Unacquire(void)
    {
        // Controller acquisition is a no-op for Xidi virtual controllers.
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::WriteEffectToFile(DirectInputDeviceType<charMode>::ConstStringType lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }


#if DIRECTINPUT_VERSION >= 0x0800
    // -------- METHODS: IDirectInputDevice8 ONLY ------------------------------ //
    // See DirectInput documentation for more information.

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::BuildActionMap(DirectInputDeviceType<charMode>::ActionFormatType* lpdiaf, DirectInputDeviceType<charMode>::ConstStringType lpszUserName, DWORD dwFlags)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::GetImageInfo(DirectInputDeviceType<charMode>::DeviceImageInfoHeaderType* lpdiDevImageInfoHeader)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    // ---------

    template <ECharMode charMode> HRESULT VirtualDirectInputDevice<charMode>::SetActionMap(DirectInputDeviceType<charMode>::ActionFormatType* lpdiActionFormat, DirectInputDeviceType<charMode>::ConstStringType lptszUserName, DWORD dwFlags)
    {
        static constexpr Message::ESeverity kMethodSeverity = Message::ESeverity::Info;
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }
#endif


    // -------- EXPLICIT TEMPLATE INSTANTIATION ---------------------------- //
    // Instantiates both the ASCII and Unicode versions of this class.

    template class VirtualDirectInputDevice<ECharMode::A>;
    template class VirtualDirectInputDevice<ECharMode::W>;
}
