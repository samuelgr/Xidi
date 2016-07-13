/*****************************************************************************
* XinputControllerDirectInput
*      Hook and helper for older DirectInput games.
*      Fixes issues associated with certain Xinput-based controllers.
*****************************************************************************
* Authored by Samuel Grossman
* Copyright (c) 2016
*****************************************************************************
* Mapper.h
*      Abstract base class for supported control mapping schemes.
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
        typedef USHORT TInstanceType;

        // Specifies the type to use for uniquely identifying a controller element.
        // TInstanceIdx and TInstanceType are combined into a single value of this type.
        typedef ULONG TInstance;


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
        protected:
            // -------- INSTANCE VARIABLES --------------------------------------------- //

            // Maps from instance identifier to base offset in the application-specified data format.
            std::unordered_map<TInstance, DWORD> instanceToOffset;

            // Maps from base offset in the application-specified data format to instance identifier.
            std::unordered_map<DWORD, TInstance> offsetToInstance;


        public:
            // -------- CLASS METHODS -------------------------------------------------- //

            // Helper to combine an instance type and index into an instance identifier.
            static inline TInstance InstanceTypeAndIndexToIdentifier(TInstanceType type, TInstanceIdx idx)
            {
                return (TInstance)(type << (8 * sizeof(TInstanceIdx))) | idx;
            }

            // Helper to extract the instance type from an instance identifier.
            static inline TInstanceType InstanceTypeFromIdentifier(TInstance id)
            {
                return (TInstanceType)(id >> (8 * sizeof(TInstanceIdx)));
            }

            // Helper to extract the instance index from an index identifier.
            static inline TInstanceIdx InstanceIndexFromIdentifier(TInstance id)
            {
                return (TInstanceIdx)(id & ((8 * sizeof(TInstanceIdx)) - 1));
            }


            // -------- INSTANCE METHODS ----------------------------------------------- //

            //


            // -------- ABSTRACT INSTANCE METHODS -------------------------------------- //

            // Parses an application-supplied DirectInput data format.
            // Return code will either be DI_OK (succeeded) or DIERR_INVALIDPARAM (failed due to an issue with the proposed data format).
            // Must be implemented by subclasses.
            virtual HRESULT ParseApplicationDataFormat(LPCDIDATAFORMAT lpdf) = 0;
        };
    }
}
