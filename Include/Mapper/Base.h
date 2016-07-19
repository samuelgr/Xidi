/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Base.h
 *      Abstract base class for supported control mapping schemes.
 *      Provides common implementations of most core functionality.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "XInputController.h"

#include <unordered_map>
#include <Xinput.h>


namespace Xidi
{
    namespace Mapper
    {
        // Specifies the type to use for identifying controller element instance numbers.
        // Valid indices are numbered from 0 and must be positive; negatives used as return codes represent invalid indices, typically indicating some kind of error.
        typedef SHORT TInstanceIdx;
        
        // Specifies the type to use for identifying controller element types.
        // Valid types are numbered from 0 and must be positive; negatives used as return codes represent invalid types, typically indicating some kind of error.
        typedef SHORT TInstanceType;
        
        // Specifies the type to use for uniquely identifying a controller element.
        // TInstanceIdx and TInstanceType are combined into a single value of this type.
        // Valid identifiers are positive; negatives used as return codes represent invalid identifiers, typically indicating some kind of error.
        typedef LONG TInstance;
        
        // Specifies the type to use for counting numbers of instances.
        typedef TInstanceIdx TInstanceCount;
        
        // Enumerates supported types of elements to be the targets of mapping.
        enum EInstanceType : TInstanceType
        {
            InstanceTypeAxis                    = 0,
            InstanceTypePov                     = 1,
            InstanceTypeButton                  = 2,
            
            InstanceTypeCount                   = 3
        };

        // Holds all properties required to configure an axis (range, deadzone, and saturation).
        // See DirectInput documentation for more information on the meaning of each field.
        struct SAxisProperties
        {
            LONG rangeMin;
            LONG rangeMax;
            DWORD deadzone;
            DWORD saturation;
        };
        
        
        // Abstract base class representing a mapped controller to the application.
        // Subclasses define the button layout to present to the application and convert data received from a Controller to the format requested by the application.
        class Base
        {
        public:
            // -------- CONSTANTS ------------------------------------------------------ //
            
            // Specifies the default minimum axis range value (based on DirectInput documentation).
            static const LONG kDefaultAxisRangeMin = 0x00000000;
            
            // Specifies the default maximum axis range value (based on DirectInput documentation).
            static const LONG kDefaultAxisRangeMax = 0x0000ffff;
            
            // Specifies the default axis deadzone (based on DirectInput documentation).
            static const DWORD kDefaultAxisDeadzone = 0;
            
            // Specifies the default axis saturation (based on DirectInput documentation).
            static const DWORD kDefaultAxisSaturation = 10000;

            // Specifies the minimum axis deadzone and saturation (based on DirectInput documentation).
            static const DWORD kMinAxisDeadzoneSaturation = 0;

            // Specifies the maximum axis deadzone and saturation (based on DirectInput documentation).
            static const DWORD kMaxAxisDeadzoneSaturation = 10000;

            // Specifies the maximum size of an application data packet, in bytes.
            // Value is equal to 1MB.
            static const DWORD kMaxDataPacketSize = 1048576;
            
            
        private:
            // -------- INSTANCE VARIABLES --------------------------------------------- //
            
            // Holds the properties of all axes present in this mapper.
            SAxisProperties* axisProperties;
            
            // Specifies the size of an application data packet, in bytes.
            DWORD dataPacketSize;

            // Maps from instance identifier to base offset in the application-specified data format.
            std::unordered_map<TInstance, DWORD> instanceToOffset;
            
            // Specifies if the maps have been initialized and contain valid data.
            BOOL mapsValid;
            
            // Maps from base offset in the application-specified data format to instance identifier.
            std::unordered_map<DWORD, TInstance> offsetToInstance;
            
            
        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
            
            // Default constructor.
            Base();
            
            // Default destructor.
            ~Base();
            
            
            // -------- CLASS METHODS -------------------------------------------------- //
            
            // Helper to combine an instance type and index into an instance identifier.
            static inline TInstance MakeInstanceIdentifier(const EInstanceType type, const TInstanceIdx idx)
            {
                return (TInstance)((TInstanceType)type << (8 * sizeof(TInstanceIdx))) | idx;
            }
            
            // Helper to extract the instance type from an instance identifier.
            static inline EInstanceType ExtractIdentifierInstanceType(const TInstance id)
            {
                return (EInstanceType)(id >> (8 * sizeof(TInstanceIdx)));
            }
            
            // Helper to extract the instance index from an index identifier.
            static inline TInstanceIdx ExtractIdentifierInstanceIndex(const TInstance id)
            {
                return (TInstanceIdx)(id & ((8 * sizeof(TInstanceIdx)) - 1));
            }
            
            // Specifies the number of bytes consumed by an instance of an object of the specified type.
            static DWORD SizeofInstance(const EInstanceType type);
            
            
        private:
            // -------- HELPERS -------------------------------------------------------- //
            
            // Returns a friendly name string for the specified axis type by GUID.
            // This is the non-Unicode version.
            LPCSTR AxisTypeToStringA(REFGUID axisTypeGUID);

            // Returns a friendly name string for the specified axis type by GUID.
            // This is the Unicode version.
            LPWSTR AxisTypeToStringW(REFGUID axisTypeGUID);
            
            // Given an array of offsets and a count, checks that they are all unset (FALSE).
            // If they are all unset, sets them (to TRUE) and returns TRUE.
            // Otherwise, leaves them alone and returns FALSE.
            BOOL CheckAndSetOffsets(BOOL* base, const DWORD count);
            
            // Given a DirectInput object instance info structure pointer, instance type, and instance number, fills the structure appropriately.
            // This is the non-Unicode version.
            void FillObjectInstanceInfoA(LPDIDEVICEOBJECTINSTANCEA instanceInfo, EInstanceType instanceType, TInstanceIdx instanceNumber);

            // Given a DirectInput object instance info structure pointer, instance type, and instance number, fills the structure appropriately.
            // This is the Unicode version.
            void FillObjectInstanceInfoW(LPDIDEVICEOBJECTINSTANCEW instanceInfo, EInstanceType instanceType, TInstanceIdx instanceNumber);
            
            // Initializes all axis properties. Idempotent; intended for lazy instantiation on first access.
            void InitializeAxisProperties(void);

            // Given a DirectInput-style identifier (combination of DIDFT_* flags), provides a mapper-style identifier.
            // Returns a negative identifier in the event of an error.
            TInstance InstanceIdentifierFromDirectInputIdentifier(DWORD diIdentifier);

            // Given a DirectInput-style instance specification (dwObj, dwHow), provides a mapper-style identifier.
            // Returns a negative identifier in the event of an error.
            TInstance InstanceIdentifierFromDirectInputSpec(DWORD dwObj, DWORD dwHow);

            // Inverts the direction of an axis reading, given a value and original range.
            LONG InvertAxisValue(LONG originalValue, LONG rangeMin, LONG rangeMax);

            // Adds a mapping between a specific instance and offset.
            void MapInstanceAndOffset(TInstance instance, DWORD offset);
            
            // Maps a value from one range to another.
            // Does not check for range errors; garbage in results in garbage out.
            LONG MapValueInRangeToRange(const LONG originalValue, const LONG originalMin, const LONG originalMax, const LONG newMin, const LONG newMax);
            
            // Given an instance type, list of instances that are used, number of instances in total, and a desired instance to select, attempts to select that instance.
            // Checks that the specified instance (by index) is currently unset (FALSE) and, if so, sets it (to TRUE).
            // If this operation succeeds, makes and returns an instance identifier using the type and index.
            // Otherwise, returns -1 cast to an instance identifier type.
            TInstance SelectInstance(const EInstanceType instanceType, BOOL* instanceUsed, const TInstanceCount instanceCount, const TInstanceIdx instanceToSelect);

            // Given an axis instance, scaled axis value, and application data structure base address, writes the value for an axis into the application data structure.
            // Applies axis properties like saturation and and deadzone, but assumes the input value is already in range.
            void WriteAxisValueToApplicationDataStructure(const TInstance axisInstance, const LONG value, LPVOID appData);

            // Given a button instance, value, and application data structure base address, writes the value for a button into the application data structure.
            // Button value should be nonzero if the button is pressed, zero otherwise.
            void WriteButtonValueToApplicationDataStructure(const TInstance buttonInstance, const BYTE value, LPVOID appData);

            // Given a POV instance, value, and application data structure base address, writes the value for a POV into the application data structure.
            // Performs no processing on the value, so assumes it is already correct for DirectInput style.
            void WritePovValueToApplicationDataStructure(const TInstance povInstance, const LONG value, LPVOID appData);
            
            
        public:
            // -------- INSTANCE METHODS ----------------------------------------------- //
            
            // Enumerates objects present in the mapping in the way DirectInput would.
            // Intended to replace VersionedIDirectInputDevice's EnumObjects method.
            HRESULT EnumerateMappedObjects(BOOL useUnicode, LPDIENUMDEVICEOBJECTSCALLBACK appCallback, LPVOID appCbParam, DWORD enumerationFlags);
            
            // Fills in a DirectInput device capabilities structure with information about the mapped game controller's buttons and axes.
            // Intended to be invoked with a structure pre-filled with other device information from VersionedIDirectInputDevice's GetCapabilities method.
            void FillDeviceCapabilities(LPDIDEVCAPS lpDIDevCaps);

            // Fills in a DirectInput object information structure with information about a specific object of the mapped game controller.
            // Corresponds directly to VersionedIDirectInputDevice's GetObjectInfo method.
            HRESULT GetMappedObjectInfo(BOOL useUnicode, LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow);

            // Retrieves a DirectInput property that this mapper is intended to intercept and handle.
            // Corresponds directly to VersionedIDirectInputDevice's GetProperty method for those properties handled by the mapper (see IsPropertyHandledByMapper).
            HRESULT GetMappedProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);
            
            // Returns the instance that corresponds to the specified offset in the application's data format.
            TInstance InstanceForOffset(DWORD offset);
            
            // Returns TRUE if the application's data format has been successfully set, FALSE otherwise.
            BOOL IsApplicationDataFormatSet(void);

            // Returns TRUE if the supplied DirectInput property is handled by the mapper, FALSE if the device should handle it directly.
            // These properties are typically accessed and mutated through VersionedIDirectInputDevice's GetProperty and SetProperty methods respectively.
            BOOL IsPropertyHandledByMapper(REFGUID guidProperty);
            
            // Returns the offset in an application's data format that corresponds to the specified instance.
            // If the offset does not exist, returns -1.
            LONG OffsetForInstance(TInstance instance);

            // Returns the offset in an application's data format that corresponds to an XInput controller element.
            // If the offset does not exist, returns -1.
            LONG OffsetForXInputControllerElement(EXInputControllerElement xElement);
            
            // Parses an application-supplied DirectInput data format.
            // Return code will either be DI_OK (succeeded) or DIERR_INVALIDPARAM (failed due to an issue with the proposed data format).
            HRESULT SetApplicationDataFormat(LPCDIDATAFORMAT lpdf);

            // Sets a DirectInput property that this mapper is intended to intercept and handle.
            // Corresponds directly to VersionedIDirectInputDevice's SetProperty method for those properties handled by the mapper (see IsPropertyHandledByMapper).
            HRESULT SetMappedProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);

            // Resets the application-supplied DirectInput data format to an uninitialized state.
            void ResetApplicationDataFormat(void);

            // Writes controller state to an application data structure, given an XInput controller's state structure.
            HRESULT WriteApplicationControllerState(XINPUT_GAMEPAD& xState, LPVOID appDataBuf, DWORD appDataSize);


            // -------- ABSTRACT INSTANCE METHODS -------------------------------------- //
            
            // Given an axis type GUID and an instance number, returns the overall instance number of that axis.
            // For example, if the GUID specifies "X axis" and the instance number is 2, returns the overall axis index of the 3rd X axis (instance numbers start at 0).
            // If the specified instance number does not exist, returns a negative value.
            // Must be implemented by subclasses.
            virtual const TInstanceIdx AxisInstanceIndex(REFGUID axisGUID, const TInstanceIdx instanceNumber) = 0;
            
            // Returns the number of axes that exist of the specified type.
            // Most controllers will return 1 if the axis type exists or 0 otherwise, since it is uncommon for a controller to support, for example, more than one X axis.
            // Must be implemented by subclasses.
            virtual const TInstanceCount AxisTypeCount(REFGUID axisGUID) = 0;

            // Given an axis instance number, returns a reference to the GUID that corresponds to the axis type.
            // For example, if the specified overall axis instance is an X axis, this method should return GUID_Xaxis.
            // Must be implemented by subclasses.
            virtual const GUID AxisTypeFromInstanceNumber(const TInstanceIdx instanceNumber) = 0;
            
            // Given an element of an XInput controller, returns the corresponding DirectInput instance.
            // Type and bounds rules are enforced: this method is called when updating controller state, and errors in the mapping will cause an error to be signalled to the application.
            // Each XInput control element may only map to a single DirectInput instance, and with one exception there may not be any overlap.
            // Triggers may both be mapped to the same axis, in which case they will share that axis. If this happens, the directionality will be determined by calling XInputTriggerSharedAxisDirection (default implementation provided, may be overridden).
            // Instance numbers for each type must be from 0 to (NumInstancesOfType - 1) for that type.
            // Additionally, types must match: XInput buttons must map to DirectInput buttons, XInput sticks must map to DirectInput axes, and the XInput dpad must map to a DirectInput POV.
            // XInput triggers, however, may map to either DirectInput axes or DirectInput buttons.
            // Subclasses may return a negative value if they wish to omit the particular XInput controller element from the mapping, in which case the application will not receive any updates for that XInput controller element.
            // Must be implemented by subclasses.
            virtual const TInstance MapXInputElementToDirectInputInstance(EXInputControllerElement element) = 0;
            
            // Specifies the number of instances that exist in the mapping of the given type.
            // For example, returns the number of buttons that exist when the input parameter is EInstanceType::InstanceTypeButton.
            // Must be implemented by subclasses.
            virtual const TInstanceCount NumInstancesOfType(const EInstanceType type) = 0;

            
            // -------- CONCRETE INSTANCE METHODS -------------------------------------- //
            
            // Called with one trigger as input when both XInput triggers map to the same shared axis.
            // If that trigger should be mapped to the negative direction of the shared axis, return a negative value.
            // Otherwise return a positive value; it is an error to return 0.
            // The default implementation maps LT to the positive direction and RT to the negative direction; this is the default behavior of an Xbox 360 controller when accessed over DirectInput.
            // May be overridden by subclasses if the default behavior is unsuitable.
            virtual LONG XInputTriggerSharedAxisDirection(EXInputControllerElement trigger);
        };
    }
}
