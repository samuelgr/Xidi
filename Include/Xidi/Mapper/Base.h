/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file Mapper/Base.h
 *   Abstract base class for supported control mapping schemes.
 *   Provides common implementations of most core functionality.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "XInputController.h"

#include <unordered_map>
#include <vector>
#include <Xinput.h>


namespace Xidi
{
    namespace Mapper
    {
        /// Specifies the type to use for identifying controller element instance numbers.
        /// Valid indices are numbered from 0 and must be positive; negatives used as return codes represent invalid indices, typically indicating some kind of error.
        typedef SHORT TInstanceIdx;
        
        /// Specifies the type to use for identifying controller element types.
        /// Valid types are numbered from 0 and must be positive; negatives used as return codes represent invalid types, typically indicating some kind of error.
        typedef SHORT TInstanceType;
        
        /// Specifies the type to use for uniquely identifying a controller element.
        /// TInstanceIdx and TInstanceType are combined into a single value of this type.
        /// Valid identifiers are positive; negatives used as return codes represent invalid identifiers, typically indicating some kind of error.
        typedef LONG TInstance;
        
        /// Specifies the type to use for counting numbers of instances.
        typedef TInstanceIdx TInstanceCount;
        
        /// Enumerates supported types of elements to be the targets of mapping.
        enum EInstanceType : TInstanceType
        {
            InstanceTypeAxis                    = 0,                        ///< Specifies an axis.
            InstanceTypePov                     = 1,                        ///< Specifies a point-of-view controller.
            InstanceTypeButton                  = 2,                        ///< Specifies a button.
            
            InstanceTypeCount                   = 3                         ///< Sentinel value.
        };

        /// Holds all properties required to configure an axis (range, deadzone, and saturation).
        /// See DirectInput documentation for more information on the meaning of each field.
        struct SAxisProperties
        {
            LONG rangeMin;                                                  ///< Minimum value of the axis range.
            LONG rangeMax;                                                  ///< Maximum value of the axis range.
            DWORD deadzone;                                                 ///< Axis deadzone.
            DWORD saturation;                                               ///< Axis saturation.
        };
        
        
        /// Abstract base class representing a mapped controller to the application.
        /// Subclasses define the button layout to present to the application and convert data received from a Controller to the format requested by the application.
        class Base
        {
        public:
            // -------- CONSTANTS ------------------------------------------ //
            
            /// Specifies the default minimum axis range value (based on DirectInput documentation).
            static const LONG kDefaultAxisRangeMin = 0x00000000;
            
            /// Specifies the default maximum axis range value (based on DirectInput documentation).
            static const LONG kDefaultAxisRangeMax = 0x0000ffff;
            
            /// Specifies the default axis deadzone (based on DirectInput documentation).
            static const DWORD kDefaultAxisDeadzone = 0;
            
            /// Specifies the default axis saturation (based on DirectInput documentation).
            static const DWORD kDefaultAxisSaturation = 10000;

            /// Specifies the minimum axis deadzone and saturation (based on DirectInput documentation).
            static const DWORD kMinAxisDeadzoneSaturation = 0;

            /// Specifies the maximum axis deadzone and saturation (based on DirectInput documentation).
            static const DWORD kMaxAxisDeadzoneSaturation = 10000;

            /// Specifies the maximum size of an application data packet, in bytes.
            /// Value is equal to 1MB.
            static const DWORD kMaxDataPacketSize = 1048576;
            
            
        private:
            // -------- TYPE DEFINITIONS ----------------------------------- //
            
            /// Internal type, used to select between Unicode and non-Unicode representations of device object instance information.
            /// Intended to be used only when enumerating device object instances.
            union UObjectInstanceInfo
            {
                DIDEVICEOBJECTINSTANCEA a;                                  ///< Non-Unicode version of the device object instance information.
                DIDEVICEOBJECTINSTANCEW w;                                  ///< Unicode version of the device object instance information.
            };
            
            
            // -------- INSTANCE VARIABLES --------------------------------- //
            
            /// Holds the properties of all axes present in this mapper.
            SAxisProperties* axisProperties;
            
            /// Cached value for the state of the XInput controller LT trigger.
            /// Used to enable shared axis updates with buffered data.
            LONG cachedValueXInputLT;

            /// Cached value for the state of the XInput controller RT trigger.
            /// Used to enable shared axis updates with buffered data.
            LONG cachedValueXInputRT;
            
            /// Specifies the size of an application data packet, in bytes.
            DWORD dataPacketSize;

            /// Maps from instance identifier to base offset in the application-specified data format.
            std::unordered_map<TInstance, DWORD> instanceToOffset;
            
            /// Specifies if the maps have been initialized and contain valid data.
            BOOL mapsValid;
            
            /// Maps from base offset in the application-specified data format to instance identifier.
            std::unordered_map<DWORD, TInstance> offsetToInstance;

            /// Holds a list of offsets for POVs that are in the application data format but do not exist on the mapping.
            /// Some games do not check how many POVs are present and assume any offset set aside for a POV will be filled with a valid reading.
            /// To accomodate this behavior, make a list of POVs that have offsets but do not exist and set them to "centered" every time device state is requested.
            std::vector<DWORD> povOffsetsToInitialize;
            
            
        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //
            
            /// Default constructor.
            Base(void);
            
            /// Default destructor.
            virtual ~Base(void);
            
            
            // -------- CLASS METHODS -------------------------------------- //
            
            /// Helper to combine an instance type and index into an instance identifier.
            /// @param [in] type Instance type.
            /// @param [in] idx Instance index.
            /// @return Combined instance identifier.
            static inline TInstance MakeInstanceIdentifier(const EInstanceType type, const TInstanceIdx idx)
            {
                return (TInstance)((TInstanceType)type << (8 * sizeof(TInstanceIdx))) | idx;
            }
            
            /// Helper to extract the instance type from an instance identifier.
            /// @param [in] id Instance identifier.
            /// @return Instance type.
            static inline EInstanceType ExtractIdentifierInstanceType(const TInstance id)
            {
                return (EInstanceType)(id >> (8 * sizeof(TInstanceIdx)));
            }
            
            /// Helper to extract the instance index from an index identifier.
            /// @param [in] id Instance identifier.
            /// @return Instance index.
            static inline TInstanceIdx ExtractIdentifierInstanceIndex(const TInstance id)
            {
                return (TInstanceIdx)(id & ((8 * sizeof(TInstanceIdx)) - 1));
            }
            
            /// Specifies the number of bytes consumed by an instance of an object of the specified type.
            /// @param [in] type Instance type.
            /// @return Number of bytes consumed by an instance of that type of object, or 0 if the type is not recognized.
            static DWORD SizeofInstance(const EInstanceType type);
            
            
        private:
            // -------- HELPERS -------------------------------------------- //
            
            /// Applies axis deadzone and saturation to a raw value within axis range.
            /// @param [in] axisInstance Instance identifier of the axis of interest.
            /// @param [in] value Raw axis value.
            /// @return Result of the calculation.
            LONG ApplyAxisPropertiesToRawValue(const TInstance axisInstance, const LONG value);
            
            /// Places a friendly name string for the specified axis type by GUID into the specified buffer.
            /// This is the non-Unicode version.
            /// @param [in] axisTypeGUID DirectInput GUID that identifies the axis type.
            /// @param [out] buf Buffer into which to write the string.
            /// @param [in] bufcount Size of the buffer, in characters.
            void AxisTypeToStringA(REFGUID axisTypeGUID, LPSTR buf, const int bufcount);

            /// Places a friendly name string for the specified axis type by GUID into the specified buffer.
            /// This is the Unicode version.
            /// @param [in] axisTypeGUID DirectInput GUID that identifies the axis type.
            /// @param [out] buf Buffer into which to write the string.
            /// @param [in] bufcount Size of the buffer, in characters.
            void AxisTypeToStringW(REFGUID axisTypeGUID, LPWSTR buf, const int bufcount);
            
            /// Given an array of offsets and a count, checks that they are all unset (`FALSE`).
            /// If they are all unset, sets them (to `TRUE`) and returns `TRUE`.
            /// Otherwise, leaves them alone and returns `FALSE`.
            /// @param [in,out] base Buffer that contains the offsets.
            /// @param [in] count Number of offsets in the buffer.
            /// @return Result of the computation (see above).
            BOOL CheckAndSetOffsets(BOOL* base, const DWORD count);
            
            /// Given a DirectInput object instance info structure pointer, instance type, and instance number, fills the structure appropriately.
            /// This is the non-Unicode version.
            /// @param [out] instanceInfo DirectInput object instance information structure. Refer to DirectInput documentation for more.
            /// @param [in] instanceType Type of instance being queried.
            /// @param [in] instanceNumber Index of the instance being queried.
            void FillObjectInstanceInfoA(LPDIDEVICEOBJECTINSTANCEA instanceInfo, EInstanceType instanceType, TInstanceIdx instanceNumber);

            /// Given a DirectInput object instance info structure pointer, instance type, and instance number, fills the structure appropriately.
            /// This is the Unicode version.
            /// @param [out] instanceInfo DirectInput object instance information structure.
            /// @param [in] instanceType Type of instance being queried.
            /// @param [in] instanceNumber Index of the instance being queried.
            void FillObjectInstanceInfoW(LPDIDEVICEOBJECTINSTANCEW instanceInfo, EInstanceType instanceType, TInstanceIdx instanceNumber);
            
            /// Initializes all axis properties. Idempotent; intended for lazy instantiation on first access.
            void InitializeAxisProperties(void);

            /// Given a DirectInput-style identifier (combination of DIDFT_* flags), provides a mapper-style identifier.
            /// Returns a negative identifier in the event of an error.
            /// @param [in] diIdentifier DirectInput-style instance identifier. Refer to DirectInput documentation for more.
            /// @return Instance identifier, or -1 if the identified instance does not exist in the mapping.
            TInstance InstanceIdentifierFromDirectInputIdentifier(DWORD diIdentifier);

            /// Given a DirectInput-style instance specification (dwObj, dwHow), provides a mapper-style identifier.
            /// Returns a negative identifier in the event of an error.
            /// @param [in] dwObj DirectInput object specification. Refer to DirectInput documentation for more.
            /// @param [in] dwHow Method by which to identify the object given its specification value. Refer to DirectInput documentation for more.
            /// @return Instance identifier, or -1 if no matching instance could be located.
            TInstance InstanceIdentifierFromDirectInputSpec(DWORD dwObj, DWORD dwHow);

            /// Inverts the direction of an axis reading, given a value and original range.
            /// @param [in] originalValue Original axis value.
            /// @param [in] rangeMin Minimum value of the axis range.
            /// @param [in] rangeMax Maximum value of the axis range.
            /// @return Result of the computation.
            LONG InvertAxisValue(LONG originalValue, LONG rangeMin, LONG rangeMax);

            /// Adds a mapping between a specific instance and offset.
            /// @param [in] instance Instance identifier.
            /// @param [in] offset Corresponding offset within the application's data format.
            void MapInstanceAndOffset(TInstance instance, DWORD offset);
            
            /// Maps a value from one range to another.
            /// Does not check for range errors; garbage in results in garbage out.
            /// @param [in] originalValue Original value within the original range.
            /// @param [in] originalMin Original range minimum value.
            /// @param [in] originalMax Original range maximum value.
            /// @param [in] newMin New range minimum value.
            /// @param [in] newMax New range maximum value.
            /// @return Result of the computation.
            LONG MapValueInRangeToRange(const LONG originalValue, const LONG originalMin, const LONG originalMax, const LONG newMin, const LONG newMax);
            
            /// Attempts to select an instance to use for a DirectInput object while parsing an application-supplied data format.
            /// Checks that the specified instance (by index) is currently unset (`FALSE`) and, if so, sets it (to `TRUE`).
            /// @param [in] instanceType Instance type of interest.
            /// @param [in,out] instanceUsed Flags that indicate whether or not other instances are currently in use by other DirectInput objects.
            /// @param [in] instanceCount Number of instances in the array of flags.
            /// @param [in] instanceToSelect Index of the desired instance that should be selected.
            /// @return Newly-created instance identifier on success, -1 on failure.
            TInstance SelectInstance(const EInstanceType instanceType, BOOL* instanceUsed, const TInstanceCount instanceCount, const TInstanceIdx instanceToSelect);

            /// Writes the value for an axis into the application data structure.
            /// Applies axis properties like saturation and and deadzone, but assumes the input value is already in range.
            /// @param [in] axisInstance Instance identifier of the axis of interest.
            /// @param [in] value Axis value to write, within the axis range, but without saturation or deadzone computations.
            /// @param [out] appData Application data buffer to which the axis value should be written.
            void WriteAxisValueToApplicationDataStructure(const TInstance axisInstance, const LONG value, LPVOID appData);

            /// Writes the value for a button into the application data structure.
            /// Button value should be nonzero if the button is pressed, zero otherwise.
            /// @param [in] buttonInstance Instance identifier of the button of interest.
            /// @param [in] value Button value to write.
            /// @param [out] appData Application data buffer to which the button value should be written.
            void WriteButtonValueToApplicationDataStructure(const TInstance buttonInstance, const BYTE value, LPVOID appData);
            
            /// Writes the value for a POV into the application data structure.
            /// Performs no processing on the value, so assumes it is already correct for DirectInput style.
            /// @param [in] povInstance Instance identifier of the POV of interest.
            /// @param [in] value POV value to write.
            /// @param [out] appData Application data buffer to which the POV value should be written.
            void WritePovValueToApplicationDataStructure(const TInstance povInstance, const LONG value, LPVOID appData);

            /// Writes the specified `LONG` value to the specified offset of the application data structure.
            /// @param [in] value Value to write.
            /// @param [in] offset Offset within the application.
            /// @param [out] appData Application data structure.
            void WriteValueToApplicationOffset(const LONG value, const DWORD offset, LPVOID appData);
            
            /// Writes the specified `BYTE` value to the specified offset of the application data structure.
            /// @param [in] value Value to write.
            /// @param [in] offset Offset within the application.
            /// @param [out] appData Application data structure.
            void WriteValueToApplicationOffset(const BYTE value, const DWORD offset, LPVOID appData);
            
            
        public:
            // -------- INSTANCE METHODS ----------------------------------- //
            
            /// Enumerates objects present in the mapping in the way DirectInput would.
            /// Intended to replicate IDirectInputDevice's EnumObjects method.
            /// @param [in] useUnicode Specifies if the implementation should fill strings with Unicode.
            /// @param [in] appCallback Callback function that should be invoked with each object's information. Refer to DirectInput documentation for more.
            /// @param [in] appCbParam Parameter to the callback function. Refer to DirectInput documentation for more.
            /// @param [in] enumerationFlags Flags used to control the enumeration. Refer to DirectInput documentation for more.
            /// @return `DI_OK` on success, `DIERR_INVALIDPARAM` if the application incorrectly responds to callbacks.
            HRESULT EnumerateMappedObjects(BOOL useUnicode, LPDIENUMDEVICEOBJECTSCALLBACK appCallback, LPVOID appCbParam, DWORD enumerationFlags);
            
            /// Fills in a DirectInput device capabilities structure with information about the mapped game controller's buttons and axes.
            /// Intended to be invoked with a structure pre-filled with other device information from the IDirectInputDevice GetCapabilities method.
            /// @param [out] lpDIDevCaps DirectInput capability structure to fill. Refer to DirectInput documentation for more.
            void FillDeviceCapabilities(LPDIDEVCAPS lpDIDevCaps);

            /// Fills in a DirectInput object information structure with information about a specific object of the mapped game controller.
            /// Corresponds directly to the IDirectInputDevice GetObjectInfo method.
            /// @param [in] useUnicode Specifies if the implementation should fill strings with Unicode.
            /// @param [out] pdidoi DirectInput object information structure to be filled.
            /// @param [in] dwObj DirectInput object specification. Refer to DirectInput documentation for more.
            /// @param [in] dwHow Method by which to identify the object given its specification value. Refer to DirectInput documentation for more.
            /// @return `DI_OK` on success or something else on error. Same return codes as IDirectInputDevice GetObjectInfo method.
            HRESULT GetMappedObjectInfo(BOOL useUnicode, LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow);

            /// Retrieves a DirectInput property that this mapper is intended to intercept and handle.
            /// Corresponds directly to the IDirectInputDevice GetProperty method for those properties handled by the mapper (see #IsPropertyHandledByMapper).
            /// @param [in] rguidProp DirectInput GUID of the property.
            /// @param [in] pdiph DirectInput property value information. Refer to DirectInput documentation for more.
            /// @return `DI_OK` on success or something else on error. Same return codes as IDirectInputDevice GetProperty method.
            HRESULT GetMappedProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);
            
            /// Specifies the instance that corresponds to the specified offset in the application's data format.
            /// @param [in] offset Offset within the application's data format.
            /// @return Corresponding instance identifier, or -1 if either the data format is not set or no instance exists at the specified offset.
            TInstance InstanceForOffset(DWORD offset);
            
            /// Specifies if the application's data format is set.
            /// @return `TRUE` if the application's data format has been successfully set, `FALSE` otherwise.
            BOOL IsApplicationDataFormatSet(void);
            
            /// Specifies if the DirectInput device property identified by the specified GUID is one that the mapper handles.
            /// @param [in] GUID of the property of interest.
            /// @return `TRUE` if the mapper handles the specified property, `FALSE` otherwise.
            BOOL IsPropertyHandledByMapper(REFGUID guidProperty);
            
            /// Specifies the offset within an application's data format that corresponds to the specified instance.
            /// @param [in] instance Instance identifier of the instance of interest.
            /// @return Offset, in bytes, or -1 if the instance is not part of the data format.
            LONG OffsetForInstance(TInstance instance);
            
            /// Specifies the offset within an application's data format that corresponds to the supplied XInput controller element.
            /// @param [in] xElement XInput controller element.
            /// @return Offeset, in bytes, or -1 if the controller element is not part of the data format.
            LONG OffsetForXInputControllerElement(EXInputControllerElement xElement);
            
            /// Parses an application-supplied DirectInput data format and sets internal data structures accordingly.
            /// @param [in] Data format specification. Refer to DirectInput documentation for more information.
            /// @return `DI_OK` on success, `DIERR_INVALIDPARAM` if the data format was rejected.
            HRESULT SetApplicationDataFormat(LPCDIDATAFORMAT lpdf);

            /// Sets a DirectInput property that this mapper is intended to intercept and handle.
            /// Corresponds directly to the IDirectInputDevice SetProperty method for those properties handled by the mapper (see #IsPropertyHandledByMapper).
            /// @param [in] rguidProp DirectInput GUID of the property.
            /// @param [in] pdiph DirectInput property value information. Refer to DirectInput documentation for more.
            /// @return `DI_OK` on success or something else on error. Same return codes as IDirectInputDevice SetProperty method.
            HRESULT SetMappedProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);

            /// Resets the application-supplied DirectInput data format to an uninitialized state.
            void ResetApplicationDataFormat(void);

            /// Writes buffered data to an application data buffer.
            /// @param [in] xController XInput controller object whose data buffer should be consumed.
            /// @param [in] appEventBuf Application data buffer. Refer to DirectInput documentation for more.
            /// @param [in,out] eventCount On input, number of events that fit in the application data buffer. On output, number of events written to the application data buffer.
            /// @param [in] peek `TRUE` if events should be left in the buffer once consumed, `FALSE` if they should be removed from it.
            /// @return `DI_OK` on success, `DI_BUFFEROVERFLOW` if the buffer has overflowed, or any other standard DirectInput return code for IDirectInputDevice GetDeviceData.
            HRESULT WriteApplicationBufferedEvents(XInputController* xController, LPDIDEVICEOBJECTDATA appEventBuf, DWORD& eventCount, const BOOL peek);
            
            /// Writes controller state to an application data structure, given an XInput controller's state structure.
            /// @param [in] xState XInput controller state information.
            /// @param [out] appDataBuf Application data buffer to which to write.
            /// @param [in] appDataSize Size of the application data buffer, in bytes. Must be at least as large as the application's data packet size, which is set when the application specifies its data format.
            /// @return `DI_OK` on success, another error code on failure.
            HRESULT WriteApplicationControllerState(XINPUT_GAMEPAD& xState, LPVOID appDataBuf, DWORD appDataSize);


            // -------- ABSTRACT INSTANCE METHODS -------------------------- //
            
            /// Given an axis type GUID and an instance number, returns the overall instance number of that axis.
            /// For example, if the GUID specifies "X axis" and the instance number is 2, returns the overall axis index of the 3rd X axis (instance numbers start at 0).
            /// If the specified instance number does not exist, returns a negative value.
            /// Must be implemented by subclasses.
            /// @param [in] axisGUID GUID that identifies the DirectInput axis of interest.
            /// @param [in] instanceNumber Instance number, in the form of a zero-based index, of the axis of interest of the specified type.
            /// @return Corresponding overall axis instance index, or negative if the specified instance does not exist in the mapping.
            virtual const TInstanceIdx AxisInstanceIndex(REFGUID axisGUID, const TInstanceIdx instanceNumber) = 0;
            
            /// Returns the number of axes that exist of the specified type.
            /// Most controllers will return 1 if the axis type exists or 0 otherwise, since it is uncommon for a controller to support, for example, more than one X axis.
            /// Must be implemented by subclasses.
            /// @param [in] axisGUID GUID that identifies the DirectInput axis of interest.
            virtual const TInstanceCount AxisTypeCount(REFGUID axisGUID) = 0;

            /// Given an axis instance number, returns a reference to the GUID that corresponds to the axis type.
            /// For example, if the specified overall axis instance is an X axis, this method should return GUID_Xaxis.
            /// Must be implemented by subclasses.
            /// @param [in] instanceNumber Axis instance number.
            /// @return GUID representing the DirectInput axis type that corresponds to the supplied axis index.
            virtual const GUID AxisTypeFromInstanceNumber(const TInstanceIdx instanceNumber) = 0;
            
            /// Given an element of an XInput controller, returns the corresponding DirectInput instance.
            /// Type and bounds rules are enforced: this method is called when updating controller state, and errors in the mapping will cause an error to be signalled to the application.
            /// Each XInput control element may only map to a single DirectInput instance, and with one exception there may not be any overlap.
            /// Triggers may both be mapped to the same axis, in which case they will share that axis. If this happens, the directionality will be determined by calling #XInputTriggerSharedAxisDirection (default implementation provided, may be overridden).
            /// Instance numbers for each type must be from 0 to (NumInstancesOfType - 1) for that type.
            /// Additionally, types must match: XInput buttons must map to DirectInput buttons, XInput sticks must map to DirectInput axes, and the XInput dpad must map to a DirectInput POV.
            /// XInput triggers, however, may map to either DirectInput axes or DirectInput buttons.
            /// Subclasses may return a negative value if they wish to omit the particular XInput controller element from the mapping, in which case the application will not receive any updates for that XInput controller element.
            /// Must be implemented by subclasses.
            /// @param [in] XInput controller element being queried.
            /// @return Instance type and index to which the specified controller element should be mapped.
            virtual const TInstance MapXInputElementToDirectInputInstance(EXInputControllerElement element) = 0;
            
            /// Specifies the number of instances that exist in the mapping of the given type.
            /// For example, returns the number of buttons that exist when the input parameter is EInstanceType::InstanceTypeButton.
            /// Must be implemented by subclasses.
            /// @param [in] type Instance type being queried.
            /// @return Number of instance of the specified type that exist in the mapping.
            virtual const TInstanceCount NumInstancesOfType(const EInstanceType type) = 0;

            
            // -------- CONCRETE INSTANCE METHODS -------------------------- //
            
            /// Called with one trigger as input when both XInput triggers map to the same shared axis.
            /// If that trigger should be mapped to the negative direction of the shared axis, return a negative value.
            /// Otherwise return a positive value; it is an error to return 0.
            /// The default implementation maps LT to the positive direction and RT to the negative direction; this is the default behavior of an Xbox 360 controller when accessed over DirectInput.
            /// May be overridden by subclasses if the default behavior is unsuitable.
            /// @param [in] XInput controller element that specifies which of the LT or RT triggers are being queried.
            /// @return Value, either positive or negative but not zero, indicating the desired direction of the specified trigger on the shared axis.
            virtual LONG XInputTriggerSharedAxisDirection(EXInputControllerElement trigger);
        };
    }
}
