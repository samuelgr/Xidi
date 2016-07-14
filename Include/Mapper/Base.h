/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Base.h
 *      Abstract base class for supported control mapping schemes.
 *      Provides common implementations of most core functionality.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput8.h"

#include <unordered_map>


namespace XinputControllerDirectInput
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
        
        
        // Abstract base class representing a mapped controller to the application.
        // Subclasses define the button layout to present to the application and convert data received from a Controller to the format requested by the application.
        class Base
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------------------- //
            
            // Maps from instance identifier to base offset in the application-specified data format.
            std::unordered_map<TInstance, DWORD> instanceToOffset;
            
            // Maps from base offset in the application-specified data format to instance identifier.
            std::unordered_map<DWORD, TInstance> offsetToInstance;
            
            // Specifies if the maps have been initialized and contain valid data.
            BOOL mapsValid;
            
            
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
            
            
        public:
            // -------- INSTANCE METHODS ----------------------------------------------- //
            
            // Enumerates objects present in the mapping in the way DirectInput would.
            // Intended to replace IDirectInputDevice8's EnumObjects method.
            HRESULT EnumerateMappedObjects(LPDIENUMDEVICEOBJECTSCALLBACK appCallback, LPVOID appCbParam, DWORD enumerationFlags);
            
            // Fills in a DirectInput device capabilities structure with information about the mapped game controller's buttons and axes.
            // Intended to be invoked with a structure pre-filled with other device information from IDirectInputDevice8's GetCapabilities method.
            void FillDeviceCapabilities(LPDIDEVCAPS lpDIDevCaps);
            
            // Returns TRUE if the application's data format has been successfully set, FALSE otherwise.
            BOOL IsApplicationDataFormatSet(void);
            
            // Parses an application-supplied DirectInput data format.
            // Return code will either be DI_OK (succeeded) or DIERR_INVALIDPARAM (failed due to an issue with the proposed data format).
            HRESULT SetApplicationDataFormat(LPCDIDATAFORMAT lpdf);


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
            virtual GUID AxisTypeFromInstanceNumber(TInstanceIdx instanceNumber) = 0;
            
            // Specifies the number of instances that exist in the mapping of the given type.
            // For example, returns the number of buttons that exist when the input parameter is EInstanceType::InstanceTypeButton.
            // Must be implemented by subclasses.
            virtual const TInstanceCount NumInstancesOfType(const EInstanceType type) = 0;
        };
    }
}
