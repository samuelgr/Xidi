/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ApiXidi.cpp
 *   Implementation of common parts of the internal API for communication
 *   between Xidi modules.
 *****************************************************************************/

#include "ApiXidi.h"
#include "Message.h"
#include "Strings.h"

#include <unordered_map>


namespace Xidi
{
    namespace Api
    {
        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Contains and allows internal access to the interface object registry.
        /// This style of implementation ensures the registry is valid early during static initialization.
        /// @return Writable reference to the registry.
        static std::unordered_map<EClass, IXidi*>& GetInterfaceObjectRegistry(void)
        {
            static std::unordered_map<EClass, IXidi*> interfaceObjectRegistry;
            return interfaceObjectRegistry;
        }

        /// Looks up and returns a pointer to the interface object corresponding to the specified class enumerator.
        /// @param [in] apiClass API class enumerator.
        /// @return Pointer to the registered implementing object, or `nullptr` if the interface is not implemented.
        static inline IXidi* LookupInterfaceObjectForClass(EClass apiClass) {
            std::unordered_map<EClass, IXidi*>& interfaceObjectRegistry = GetInterfaceObjectRegistry();

            if (false == interfaceObjectRegistry.contains(apiClass))
                return nullptr;

            return interfaceObjectRegistry[apiClass];
        }

        /// Registers an interface object as the implementing object for the Xidi API of the specified class.
        /// If another object is already registered, this function does nothing.
        /// @param [in] apiClass API class enumerator.
        /// @param [in] interfaceObject Pointer to the interface object to register.
        static inline void RegisterInterfaceObject(EClass apiClass, IXidi* interfaceObject)
        {
            std::unordered_map<EClass, IXidi*>& interfaceObjectRegistry = GetInterfaceObjectRegistry();

            if (false == interfaceObjectRegistry.contains(apiClass))
                interfaceObjectRegistry[apiClass] = interfaceObject;
        }


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //
        // See "ApiXidi.h" for documentation.

        IXidi::IXidi(EClass apiClass)
        {
            RegisterInterfaceObject(apiClass, this);
        }


        // -------- XIDI API ----------------------------------------------- //

        /// Implements the Xidi API interface #IMetadata.
        class MetadataProvider : public IMetadata
        {
        public:
            // -------- CONCRETE INSTANCE METHODS -------------------------- //
            // See "ApiXidi.h" for documentation.

            IMetadata::SVersion GetVersion(void) const override
            {
                return {GIT_VERSION_STRUCT, Strings::kStrVersion};
            }

            // --------

            std::wstring_view GetFormName(void) const override
            {
                return Strings::kStrFormName;
            }
        };

        /// Singleton Xidi API implementation object.
        static MetadataProvider metadataProvider;
    }
}


// -------- FUNCTIONS ------------------------------------------------------ //
// See "ApiXidi.h" for more information.

extern "C" __declspec(dllexport) void* XidiApiGetInterface(Xidi::Api::EClass apiClass)
{
    return LookupInterfaceObjectForClass(apiClass);
}
