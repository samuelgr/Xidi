/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file DataFormat.cpp
 *   Implementation of all functionality related to parsing and formatting
 *   controller data using the format specified by a DirectInput application.
 *****************************************************************************/

#include "ApiBitSet.h"
#include "ApiDirectInput.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "Message.h"
#include "Strings.h"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <vector>


// Handler for invalid or unselectable object data format specifications.
// If the object specification marks the object as optional, it is skipped. Otherwise, the entire application data format is rejected.
// Intended to be used within #DataFormat::CreateFromApplicationFormatSpec and tied to its implementation.
#define DATAFORMAT_HANDLE_INVALID_OBJECT_SPEC(objectFormatSpec, objectIndex, reasonStr, ...) \
    if (0 != (objectFormatSpec.dwType & DIDFT_OPTIONAL)) \
    { \
        Message::OutputFormatted(Message::ESeverity::Debug, L"Skipping optional object at index %d: " reasonStr, (int)objectIndex, ##__VA_ARGS__); \
        continue; \
    } \
    else \
    { \
        Message::OutputFormatted(Message::ESeverity::Warning, L"Rejecting application data format due to non-optional object at index %d: " reasonStr, (int)objectIndex, ##__VA_ARGS__); \
        return nullptr; \
    } \


namespace Xidi
{
    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Encapsulates intermediate state and provides helpful functionality to be used while building a data format object from an application-provided data format specification.
    /// Instances of this object are in essence consumed as methods are called. When constructed all instance variables are initialized to pristine for a new application data format specification, but elements are removed and space marked allocated as methods are called.
    class DataFormatBuildHelper
    {
    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Ordered set of remaining available axes.
        BitSetEnum<Controller::EAxis> availableAxes;

        /// Ordered set of remaining available buttons.
        BitSetEnum<Controller::EButton> availableButtons;

        /// Whether or not a POV is available.
        bool availablePov;

        /// Compact representation of the allocation status of each byte offset in the application data packet.
        std::vector<bool> usedByteOffsets;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor.
        /// Uses the provided information to initialize the sets of available controller elements and determine the number of byte allocations to track.
        /// @param [in] controllerCapabilities Capabilities of the controller for which the data format is being constructed.
        /// @param [in] dataPacketSize Size, in bytes, of the application's data packet.
        DataFormatBuildHelper(const Controller::SCapabilities controllerCapabilities, TOffset dataPacketSize) : availableAxes(), availableButtons(), availablePov(), usedByteOffsets(dataPacketSize)
        {
            for (int i = 0; i < controllerCapabilities.numAxes; ++i)
                availableAxes.insert((int)controllerCapabilities.axisCapabilities[i].type);

            for (int i = 0; i < controllerCapabilities.numButtons; ++i)
                availableButtons.insert(i);

            if (true == controllerCapabilities.hasPov)
                availablePov = true;
        }


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Attempts to allocate space in the application's for a value of the specified type starting at the specified offset.
        /// Reasons for failure are that the offset + value size exceeds the bounds of the data packet or that a previous allocation overlaps with the requested allocation.
        /// @tparam ValueType Type of value for which space is being allocated, generally one of the type aliases that corresponds to values written for each controller element type.
        /// @param [in] offset Starting offset in the application's data format for where the value is intended to be written.
        /// @return `true` if the allocation was successful, `false` otherwise.
        template <typename ValueType> bool AllocateAtOffset(TOffset offset)
        {
            // Sanity check: is the requested offset properly aligned to the size of the value, as required by DirectInput documentation?
            if (0 != (offset % sizeof(ValueType)))
                return false;

            // Sanity check: does the requested allocation fit within the application's data packet?
            if ((offset + sizeof(ValueType)) > usedByteOffsets.size())
                return false;

            // Allocate the value one byte at a time, breaking if another allocation overlaps.
            for (TOffset i = 0; i < sizeof(ValueType); ++i)
            {
                if (true == usedByteOffsets[offset + i])
                    return false;

                usedByteOffsets[offset + i] = true;
            }

            return true;
        }

        /// Retrieves the next element of the specified type that is available for selection.
        /// @param [in] elementType Type of element requested.
        /// @return Identifier of the selected element, if one is available.
        std::optional<Controller::SElementIdentifier> GetNextAvailableOfType(Controller::EElementType elementType)
        {
            switch (elementType)
            {
            case Controller::EElementType::Axis:
                if (false == availableAxes.empty())
                {
                    const Controller::EAxis nextAvailableAxis = (Controller::EAxis)((int)availableAxes.front());
                    availableAxes.erase((int)nextAvailableAxis);
                    return Controller::SElementIdentifier({.type = Controller::EElementType::Axis, .axis = nextAvailableAxis});
                }
                break;

            case Controller::EElementType::Button:
                if (false == availableButtons.empty())
                {
                    const Controller::EButton nextAvailableButton = (Controller::EButton)((int)availableButtons.front());
                    availableButtons.erase((int)nextAvailableButton);
                    return Controller::SElementIdentifier({.type = Controller::EElementType::Button, .button = nextAvailableButton});
                }
                break;

            case Controller::EElementType::Pov:
                if (true == availablePov)
                {
                    availablePov = false;
                    return Controller::SElementIdentifier({.type = Controller::EElementType::Pov});
                }
                break;
            }

            return std::nullopt;
        }

        /// Retrieves a specific element that an application has requested for selection.
        /// @param [in] element Identifier of the requested element.
        /// @return Identifier of the selected element (which will be the same as the identifier passed aas a parameter), if it is available.
        std::optional<Controller::SElementIdentifier> GetSpecificElement(Controller::SElementIdentifier element)
        {
            switch (element.type)
            {
            case Controller::EElementType::Axis:
                do
                {
                    if (true == availableAxes.contains((int)element.axis))
                    {
                        availableAxes.erase((int)element.axis);
                        return element;
                    }
                } while (false);
                break;

            case Controller::EElementType::Button:
                do
                {
                    if (true == availableButtons.contains((int)element.button))
                    {
                        availableButtons.erase((int)element.button);
                        return element;
                    }
                } while (false);
                break;

            case Controller::EElementType::Pov:
                if (true == availablePov)
                {
                    availablePov = false;
                    return element;
                }
                break;
            }

            return std::nullopt;
        }
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Maps a GUID to an axis type, if one is specified.
    /// @param [in] pguid Pointer to the GUID to check.
    /// @return Corresponding axis type, if the GUID identifies a known axis type.
    static std::optional<Controller::EAxis> AxisTypeFromGuid(const GUID* pguid)
    {
        if (nullptr != pguid)
        {
            if (GUID_XAxis == *pguid)
                return Controller::EAxis::X;
            else if (GUID_YAxis == *pguid)
                return Controller::EAxis::Y;
            else if (GUID_ZAxis == *pguid)
                return Controller::EAxis::Z;
            else if (GUID_RxAxis == *pguid)
                return Controller::EAxis::RotX;
            else if (GUID_RyAxis == *pguid)
                return Controller::EAxis::RotY;
            else if (GUID_RzAxis == *pguid)
                return Controller::EAxis::RotZ;
        }

        return std::nullopt;
    }

    /// Returns a string representing the specified axis type.
    /// @param [in] axis Axis type for which a string is requested.
    /// @return String representation of the axis type.
    static const wchar_t* AxisTypeString(Controller::EAxis axis)
    {
        switch (axis)
        {
        case Controller::EAxis::X:
            return L"X";
        case Controller::EAxis::Y:
            return L"Y";
        case Controller::EAxis::Z:
            return L"Z";
        case Controller::EAxis::RotX:
            return L"RotX";
        case Controller::EAxis::RotY:
            return L"RotY";
        case Controller::EAxis::RotZ:
            return L"RotZ";
        }

        return L"(unrecognized axis)";
    }

    /// Returns a string representing the specified data format flags value.
    /// @param [in] dwFlags Flags from the data format specification structure.
    /// @return String representation of the flag value.
    static const wchar_t* DataFormatFlagsString(DWORD dwFlags)
    {
        switch (dwFlags)
        {
        case 0:
            return L"None";
        case DIDF_ABSAXIS:
            return L"DIDF_ABSAXIS";
        case DIDF_RELAXIS:
            return L"DIDF_RELAXIS";
        }

        return L"(unrecognized value)";
    }

    /// Compares the specified GUID with the known list of object unique identifiers.
    /// Returns a string that represents the specified GUID.
    /// @param [in] pguid Pointer to the GUID to check.
    /// @return String representation of the GUID's semantics, even if unknown.
    static const wchar_t* GuidTypeString(const GUID* pguid)
    {
        if (nullptr == pguid)
            return L"(null)";
        else if (GUID_XAxis == *pguid)
            return _CRT_WIDE(XIDI_AXIS_NAME_X);
        else if (GUID_YAxis == *pguid)
            return _CRT_WIDE(XIDI_AXIS_NAME_Y);
        else if (GUID_ZAxis == *pguid)
            return _CRT_WIDE(XIDI_AXIS_NAME_Z);
        else if (GUID_RxAxis == *pguid)
            return _CRT_WIDE(XIDI_AXIS_NAME_RX);
        else if (GUID_RyAxis == *pguid)
            return _CRT_WIDE(XIDI_AXIS_NAME_RY);
        else if (GUID_RzAxis == *pguid)
            return _CRT_WIDE(XIDI_AXIS_NAME_RZ);
        else if (GUID_Slider == *pguid)
            return L"Slider";
        else if (GUID_Button == *pguid)
            return L"Button";
        else if (GUID_Key == *pguid)
            return L"Key";
        else if (GUID_POV == *pguid)
            return L"POV";
        else if (GUID_Unknown == *pguid)
            return L"Unknown";
        else
            return L"(unrecognized)";
    }
    
    /// Dumps a data format specification. Intended as a debugging aid.
    /// @param [in] appFormatSpec Application-provided DirectInput data format specification.
    static void DumpDataFormatSpecification(const DIDATAFORMAT& appFormatSpec)
    {
        constexpr Message::ESeverity kDumpSeverity = Message::ESeverity::Debug;

        if (Message::WillOutputMessageOfSeverity(kDumpSeverity))
        {
            Message::Output(kDumpSeverity, L"Begin dump of data format specification.");

            // First, dump the top-level structure members along with some preliminary validity checks.
            Message::Output(kDumpSeverity, L"  Metadata:");
            Message::OutputFormatted(kDumpSeverity, L"    dwSize = %u (%s; expected %u)", appFormatSpec.dwSize, (sizeof(DIDATAFORMAT) == appFormatSpec.dwSize ? L"OK" : L"INCORRECT"), (unsigned int)sizeof(DIDATAFORMAT));
            Message::OutputFormatted(kDumpSeverity, L"    dwObjSize = %u (%s; expected %u)", appFormatSpec.dwObjSize, (sizeof(DIOBJECTDATAFORMAT) == appFormatSpec.dwObjSize ? L"OK" : L"INCORRECT"), (unsigned int)sizeof(DIOBJECTDATAFORMAT));
            Message::OutputFormatted(kDumpSeverity, L"    dwFlags = 0x%x (%s)", appFormatSpec.dwFlags, DataFormatFlagsString(appFormatSpec.dwFlags));
            Message::OutputFormatted(kDumpSeverity, L"    dwDataSize = %u (%s)", appFormatSpec.dwDataSize, (0 == appFormatSpec.dwDataSize % 4 ? L"POSSIBLY OK; is a multiple of 4" : L"INCORRECT; must be a multiple of 4"));
            Message::OutputFormatted(kDumpSeverity, L"    dwNumObjs = %u", appFormatSpec.dwNumObjs);

            // Second, dump the individual objects.
            Message::Output(kDumpSeverity, L"  Objects:");
            
            if (0 == appFormatSpec.dwNumObjs)
            {
                Message::Output(kDumpSeverity, L"    (none present)");
            }
            else if (nullptr == appFormatSpec.rgodf)
            {
                Message::OutputFormatted(kDumpSeverity, L"    (%u missing)", appFormatSpec.dwNumObjs);
            }
            else
            {
                for (DWORD i = 0; i < appFormatSpec.dwNumObjs; ++i)
                    Message::OutputFormatted(kDumpSeverity, L"    rgodf[%4u]    { pguid = %s, dwOfs = %u, dwType = 0x%08x, dwFlags = 0x%08x }", i, GuidTypeString(appFormatSpec.rgodf[i].pguid), appFormatSpec.rgodf[i].dwOfs, appFormatSpec.rgodf[i].dwType, appFormatSpec.rgodf[i].dwFlags);
            }

            Message::Output(kDumpSeverity, L"End dump of data format specification.");
        }
    }

    /// Checks if the specified GUID identifies an object of the specified type.
    /// If it does, then returns the same type that was passed in, otherwise returns nothing.
    /// @param [in] pguid Pointer to the GUID to check.
    static std::optional<Controller::EElementType> ElementTypeIfGuidMatches(const GUID* pguid, Controller::EElementType elementType)
    {
        // If a GUID is absent, it matches everything.
        if (nullptr == pguid)
            return elementType;

        // If a GUID is present, it needs to be consistent with the element type.
        switch (elementType)
        {
        case Controller::EElementType::Axis:
            if ((GUID_XAxis == *pguid) || (GUID_YAxis == *pguid) || (GUID_ZAxis == *pguid) || (GUID_RxAxis == *pguid) || (GUID_RyAxis == *pguid) || (GUID_RzAxis == *pguid))
                return elementType;
            break;

        case Controller::EElementType::Button:
            if (GUID_Button == *pguid)
                return elementType;
            break;

        case Controller::EElementType::Pov:
            if (GUID_POV == *pguid)
                return elementType;
            break;
        }

        return std::nullopt;
    }
    
    /// Determines the type of element being requested to be associated with a single object in an application's data format specification.
    /// Type can only be determined if it is recognized and there is no mismatch between the GUID and the type filter.
    /// @param [in] Individual object specification within the application's data format specification.
    /// @return Requested element type, if it is recognized and consistently specified by the object format specification.
    static std::optional<Controller::EElementType> ElementTypeFromObjectFormatSpec(const DIOBJECTDATAFORMAT& objectFormatSpec)
    {
        // Only position information can be reported by virtual controller objects.
        // It is assumed that button and POV state values are considered position information.
        switch (objectFormatSpec.dwFlags)
        {
        case 0:
        case DIDOI_ASPECTPOSITION:
            if (0 != (DIDFT_GETTYPE(objectFormatSpec.dwType) & DIDFT_ABSAXIS))
                return ElementTypeIfGuidMatches(objectFormatSpec.pguid, Controller::EElementType::Axis);
            else if (0 != (DIDFT_GETTYPE(objectFormatSpec.dwType) & DIDFT_PSHBUTTON))
                return ElementTypeIfGuidMatches(objectFormatSpec.pguid, Controller::EElementType::Button);
            else if (0 != (DIDFT_GETTYPE(objectFormatSpec.dwType) & DIDFT_POV))
                return ElementTypeIfGuidMatches(objectFormatSpec.pguid, Controller::EElementType::Pov);
            break;

        default:
            break;
        }

        return std::nullopt;
    }


    // -------- CLASS METHODS ---------------------------------------------- //
    // See "DataFormat.h" for documentation.

    std::unique_ptr<DataFormat> DataFormat::CreateFromApplicationFormatSpec(const DIDATAFORMAT& appFormatSpec, const Controller::SCapabilities controllerCapabilities)
    {
        DumpDataFormatSpecification(appFormatSpec);
        
        // Sanity check: is data packet size is a multiple of 4, as required by DirectInput?
        if (0 != (appFormatSpec.dwDataSize % 4))
        {
            Message::Output(Message::ESeverity::Warning, L"Rejecting application data format because the data packet size is not divisible by 4.");
            return nullptr;
        }

        // Sanity check: is the data packet size within bounds?
        if (kMaxDataPacketSizeBytes < appFormatSpec.dwDataSize)
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"Rejecting application data format because the data packet size is too large (%u bytes versus maximum %u bytes).", appFormatSpec.dwDataSize, kMaxDataPacketSizeBytes);
            return nullptr;
        }

        // Sanity check: are there any objects defined?
        if (appFormatSpec.dwNumObjs < 1)
        {
            Message::Output(Message::ESeverity::Warning, L"Rejecting application data format because it does not define any objects.");
            return nullptr;
        }
        else if (nullptr == appFormatSpec.rgodf)
        {
            Message::Output(Message::ESeverity::Warning, L"Rejecting application data format because it does not contain any object format specifications.");
            return nullptr;
        }

        // Sanity check: are the requested flags supported?
        // Per DirectInput documentation flags are used to specify the axis mode, and only absolute mode is supported by virtual controllers.
        switch (appFormatSpec.dwFlags)
        {
        case 0:
        case DIDF_ABSAXIS:
            break;
        default:
            Message::OutputFormatted(Message::ESeverity::Warning, L"Rejecting application data format because its flags are unsupported (0x%08x).", appFormatSpec.dwFlags);
            return nullptr;
        }

        DataFormatBuildHelper buildHelper(controllerCapabilities, appFormatSpec.dwDataSize);
        SDataFormatSpec dataFormatSpec(appFormatSpec.dwDataSize);
        int numElementsSelected = 0;

        for (DWORD i = 0; i < appFormatSpec.dwNumObjs; ++i)
        {
            const DIOBJECTDATAFORMAT& objectFormatSpec = appFormatSpec.rgodf[i];

            // Step 1
            // Figure out the type of controller element being requested by the object format specification.
            // If the request is inconsistent (i.e. mismatch between GUID identifier and type filter) or unrecognized (i.e. type filter identifies a type not supported by virtual controllers) then the object format specification is rejected.
            const std::optional<Controller::EElementType> maybeElementType = ElementTypeFromObjectFormatSpec(objectFormatSpec);
            if (false == maybeElementType.has_value())
            {
                DATAFORMAT_HANDLE_INVALID_OBJECT_SPEC(objectFormatSpec, i, L"Inconsistent or unrecognized type 0x%x, GUID %s, and flags 0x%08x.", (unsigned int)DIDFT_GETTYPE(objectFormatSpec.dwType), GuidTypeString(objectFormatSpec.pguid), objectFormatSpec.dwFlags);
            }

            // Step 2
            // Allocate space in the data packet at the specified offset and attempt to select a specific controller element, based on the capabilities of the virtual controller.
            // If allocation fails, the application data format specification is invalid and needs to be rejected outright.
            // If allocation succeeds but element selection fails, then the object format specification is rejected.
            std::optional<Controller::SElementIdentifier> maybeSelectedElement = std::nullopt;
            constexpr int kWildcardInstanceIndex = (int)DIDFT_GETINSTANCE(DIDFT_ANYINSTANCE);

            switch (maybeElementType.value())
            {
            case Controller::EElementType::Axis:
                do
                {
                    if (false == buildHelper.AllocateAtOffset<TAxisValue>(objectFormatSpec.dwOfs))
                    {
                        Message::OutputFormatted(Message::ESeverity::Warning, L"Rejecting application data format due object at index %d: Failed to allocate %d byte(s) for an axis at offset %u.", (int)i, (int)sizeof(TAxisValue), objectFormatSpec.dwOfs);
                        return nullptr;
                    }

                    // Since the GUID was already verified to be consistent with requesting an axis, either the GUID will be absent or it will identify a specific type of axis.
                    // If present, then the application is requesting a specific axis. If absent, then the application wants the next axis of any type.
                    const std::optional<Controller::EAxis> maybeAxisType = AxisTypeFromGuid(objectFormatSpec.pguid);
                    if (maybeAxisType.has_value())
                    {
                        // Only one axis of each type is available on the virtual controller.
                        // For there to be a successful element selection, the application must be requesting either the first instance or any instance of an axis of the specified type.
                        // Note that this behavior is based on the assumption that an axis instance number specified along with a GUID type is a relative constraint rather than an absolute constraint.
                        // For example, suppose the GUID is `GUID_XAxis` and the instance is number is `N`. A relative constraint says "match the Nth X axis" whereas an absolute constraint says "match the Nth axis on the controller, but only if it happens to be an X axis."
                        const int kRequestedInstanceIndex = DIDFT_GETINSTANCE(objectFormatSpec.dwType);
                        if (0 == kRequestedInstanceIndex || kWildcardInstanceIndex == kRequestedInstanceIndex)
                            maybeSelectedElement = buildHelper.GetSpecificElement({.type = Controller::EElementType::Axis, .axis = maybeAxisType.value()});
                    }
                    else
                    {
                        const int kRequestedInstanceIndex = DIDFT_GETINSTANCE(objectFormatSpec.dwType);
                        if (kWildcardInstanceIndex == kRequestedInstanceIndex)
                            maybeSelectedElement = buildHelper.GetNextAvailableOfType(Controller::EElementType::Axis);
                        else if ((kRequestedInstanceIndex >= 0) && (kRequestedInstanceIndex < _countof(controllerCapabilities.axisCapabilities)))
                            maybeSelectedElement = buildHelper.GetSpecificElement({.type = Controller::EElementType::Axis, .axis = controllerCapabilities.axisCapabilities[kRequestedInstanceIndex].type});
                    }
                } while (false);

                // For debugging.
                if (true == maybeSelectedElement.has_value())
                    Message::OutputFormatted(Message::ESeverity::Debug, L"Object at index %d: Selected %s axis for offset %u.", (int)i, AxisTypeString(maybeSelectedElement.value().axis), objectFormatSpec.dwOfs);

                break;

            case Controller::EElementType::Button:
                do
                {
                    if (false == buildHelper.AllocateAtOffset<TButtonValue>(objectFormatSpec.dwOfs))
                    {
                        Message::OutputFormatted(Message::ESeverity::Warning, L"Rejecting application data format due object at index %d: Failed to allocate %d byte(s) for a button at offset %u.", (int)i, (int)sizeof(TButtonValue), objectFormatSpec.dwOfs);
                        return nullptr;
                    }

                    // Applications can request the next available button or a specific button by number.
                    const int kRequestedInstanceIndex = DIDFT_GETINSTANCE(objectFormatSpec.dwType);
                    if (kWildcardInstanceIndex == kRequestedInstanceIndex)
                        maybeSelectedElement = buildHelper.GetNextAvailableOfType(Controller::EElementType::Button);
                    else
                        maybeSelectedElement = buildHelper.GetSpecificElement({.type = Controller::EElementType::Button, .button = (Controller::EButton)kRequestedInstanceIndex});
                } while (false);

                // For debugging.
                if (true == maybeSelectedElement.has_value())
                    Message::OutputFormatted(Message::ESeverity::Debug, L"Object at index %d: Selected button %d for offset %u.", (int)i, (1 + (int)maybeSelectedElement.value().button), objectFormatSpec.dwOfs);

                break;

            case Controller::EElementType::Pov:
                do
                {
                    if (false == buildHelper.AllocateAtOffset<EPovValue>(objectFormatSpec.dwOfs))
                    {
                        Message::OutputFormatted(Message::ESeverity::Warning, L"Rejecting application data format due object at index %d: Failed to allocate %d byte(s) for a POV at offset %u.", (int)i, (int)sizeof(EPovValue), objectFormatSpec.dwOfs);
                        return nullptr;
                    }

                    // Either zero or one POV elements exist on the virtual controller.
                    const int kRequestedInstanceIndex = DIDFT_GETINSTANCE(objectFormatSpec.dwType);
                    if (kWildcardInstanceIndex == kRequestedInstanceIndex)
                        maybeSelectedElement = buildHelper.GetNextAvailableOfType(Controller::EElementType::Pov);
                    else if (0 == kRequestedInstanceIndex)
                        maybeSelectedElement = buildHelper.GetSpecificElement({.type = Controller::EElementType::Pov});

                    // Unselected POV offsets are tracked separately because they need to be initialized to a non-zero value when writing a data packet.
                    if (false == maybeSelectedElement.has_value())
                    {
                        Message::OutputFormatted(Message::ESeverity::Debug, L"Object at index %d: Found unused POV at offset %u.", (int)i, objectFormatSpec.dwOfs);
                        dataFormatSpec.SubmitUnusedPovOffset(objectFormatSpec.dwOfs);
                    }
                } while (false);

                // For debugging.
                if (true == maybeSelectedElement.has_value())
                    Message::OutputFormatted(Message::ESeverity::Debug, L"Object at index %d: Selected POV for offset %u.", (int)i, objectFormatSpec.dwOfs);

                break;
            }

            // Step 3
            // If an element was successfully selected, it needs to be added to the data format specification.
            // Otherwise, the object format specification is rejected.
            if (true == maybeSelectedElement.has_value())
            {
                dataFormatSpec.SetOffsetForElement(maybeSelectedElement.value(), objectFormatSpec.dwOfs);
                numElementsSelected += 1;
            }
            else
            {
                DATAFORMAT_HANDLE_INVALID_OBJECT_SPEC(objectFormatSpec, i, L"No matching virtual controller element is available for selection.");
            }
        }

        Message::OutputFormatted(Message::ESeverity::Info, L"Accepted and successfully set application data format. Total data packet size is %u byte(s), and %d virtual controller element(s) were selected.", appFormatSpec.dwDataSize, numElementsSelected);
        return std::unique_ptr<DataFormat>(new DataFormat(controllerCapabilities, std::move(dataFormatSpec)));
    }

    // --------

    EPovValue DataFormat::DirectInputPovValue(Controller::UPovDirection pov)
    {
        static constexpr EPovValue kPovDirectionValues[3][3] = {
            {EPovValue::NW, EPovValue::N,       EPovValue::NE},
            {EPovValue::W,  EPovValue::Center,  EPovValue::E},
            {EPovValue::SW, EPovValue::S,       EPovValue::SE}
        };

        const int xCoord = ((true == pov.components[(int)Controller::EPovDirection::Right]) ? 1 : 0) - ((true == pov.components[(int)Controller::EPovDirection::Left]) ? 1 : 0);
        const int yCoord = ((true == pov.components[(int)Controller::EPovDirection::Down]) ? 1 : 0) - ((true == pov.components[(int)Controller::EPovDirection::Up]) ? 1 : 0);

        return kPovDirectionValues[1 + yCoord][1 + xCoord];
    }


    // -------- INSTANCE METHODS ------------------------------------------- //
    // See "DataFormat.h" for documentation.

    std::optional<Controller::SElementIdentifier> DataFormat::GetElementForOffset(TOffset offset) const
    {
        const auto elementRecord = dataFormatSpec.offsetElementMap.find(offset);
        if (elementRecord != dataFormatSpec.offsetElementMap.cend())
            return elementRecord->second;

        return std::nullopt;
    }

    // --------

    std::optional<TOffset> DataFormat::GetOffsetForElement(Controller::SElementIdentifier element) const
    {
        switch (element.type)
        {
        case Controller::EElementType::Axis:
            if (kInvalidOffsetValue != dataFormatSpec.axisOffset[(int)element.axis])
                return dataFormatSpec.axisOffset[(int)element.axis];
            break;

        case Controller::EElementType::Button:
            if (kInvalidOffsetValue != dataFormatSpec.buttonOffset[(int)element.button])
                return dataFormatSpec.buttonOffset[(int)element.button];
            break;

        case Controller::EElementType::Pov:
            if (kInvalidOffsetValue != dataFormatSpec.povOffset)
                return dataFormatSpec.povOffset;
            break;
        }
        
        return std::nullopt;
    }

    // --------

    bool DataFormat::WriteDataPacket(void* packetBuffer, TOffset packetBufferSizeBytes, const Controller::SState& controllerState) const
    {
        // Sanity check: did the application allocate sufficient buffer space?
        if (packetBufferSizeBytes < dataFormatSpec.packetSizeBytes)
            return false;

        uint8_t* const packetByteBuffer = (uint8_t*)packetBuffer;

        // Initialize the application data packet.
        // Everything not explicitly written will be 0, except for unused POVs which must be initialized to center position.
        ZeroMemory(packetBuffer, packetBufferSizeBytes);
        for (auto povOffsetUnused : dataFormatSpec.povOffsetsUnused)
        {
            EPovValue* const valueLocation = (EPovValue*)(&packetByteBuffer[povOffsetUnused]);
            *valueLocation = EPovValue::Center;
        }
        
        // Axis values
        for (int i = 0; i < _countof(dataFormatSpec.axisOffset); ++i)
        {
            if (kInvalidOffsetValue != dataFormatSpec.axisOffset[i])
            {
                TAxisValue* const valueLocation = (TAxisValue*)(&packetByteBuffer[dataFormatSpec.axisOffset[i]]);
                *valueLocation = DirectInputAxisValue(controllerState.axis[i]);
            }
        }

        // Button values
        for (int i = 0; i < _countof(dataFormatSpec.buttonOffset); ++i)
        {
            if (kInvalidOffsetValue != dataFormatSpec.buttonOffset[i])
            {
                TButtonValue* const valueLocation = (TButtonValue*)(&packetByteBuffer[dataFormatSpec.buttonOffset[i]]);
                *valueLocation = DirectInputButtonValue(controllerState.button[i]);
            }
        }

        // POV value
        if (kInvalidOffsetValue != dataFormatSpec.povOffset)
        {
            EPovValue* const valueLocation = (EPovValue*)(&packetByteBuffer[dataFormatSpec.povOffset]);
            *valueLocation = DirectInputPovValue(controllerState.povDirection);
        }
        
        return true;
    }
}
