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


        public:
            // -------- CLASS METHODS -------------------------------------------------- //

            // Helper to combine an instance type and index into an instance identifier.
            static inline TInstance MakeInstanceIdentifier(EInstanceType type, TInstanceIdx idx)
            {
                return (TInstance)((TInstanceType)type << (8 * sizeof(TInstanceIdx))) | idx;
            }

            // Helper to extract the instance type from an instance identifier.
            static inline EInstanceType ExtractIdentifierInstanceType(TInstance id)
            {
                return (EInstanceType)(id >> (8 * sizeof(TInstanceIdx)));
            }

            // Helper to extract the instance index from an index identifier.
            static inline TInstanceIdx ExtractIdentifierInstanceIndex(TInstance id)
            {
                return (TInstanceIdx)(id & ((8 * sizeof(TInstanceIdx)) - 1));
            }

            // Specifies the number of bytes consumed by an instance of an object of the specified type.
            static DWORD SizeofInstance(EInstanceType type);


            // -------- INSTANCE METHODS ----------------------------------------------- //
        private:
            BOOL CheckAndSetOffsets(BOOL* base, DWORD count);
            
            TInstance SelectInstance(EInstanceType instanceType, BOOL* instanceUsed, TInstanceCount instanceCount, TInstanceIdx instanceToSelect);
            
        public:
            // Parses an application-supplied DirectInput data format.
            // Return code will either be DI_OK (succeeded) or DIERR_INVALIDPARAM (failed due to an issue with the proposed data format).
            HRESULT ParseApplicationDataFormat(LPCDIDATAFORMAT lpdf);


            // -------- ABSTRACT INSTANCE METHODS -------------------------------------- //

            // Specifies the number of instances that exist in the mapping of the given type.
            // Must be implemented by subclasses.
            virtual TInstanceCount NumInstancesOfType(EInstanceType type) = 0;

            virtual TInstanceIdx AxisInstanceIndex(REFGUID axisGUID, DWORD instanceNumber) = 0;

            virtual BOOL AxisInstanceExists(REFGUID axisGUID, DWORD instanceNumber) = 0;

            virtual TInstanceCount AxisTypeCount(REFGUID axisGUID) = 0;
        };
    }
}
