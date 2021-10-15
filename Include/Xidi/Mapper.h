/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Mapper.h
 *   Declaration of functionality used to implement mappings of an entire
 *   XInput controller layout to a virtual controller layout.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"

#include <memory>
#include <string_view>
#include <xinput.h>


// -------- MACROS --------------------------------------------------------- //

/// Computes the index of the specified named controller element in the pointer array of the element map.
#define ELEMENT_MAP_INDEX_OF(element)       ((unsigned int)(offsetof(::Xidi::Controller::Mapper::UElementMap, named.##element) / sizeof(::Xidi::Controller::Mapper::UElementMap::all[0])))


namespace Xidi
{
    namespace Controller
    {
        /// Maps an XInput controller layout to a virtual controller layout.
        /// Each instance of this class represents a different virtual controller layout.
        class Mapper
        {
        public:
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// XInput controller element mappers, one per controller element.
            /// For controller elements that are not used, a value of `nullptr` may be used instead.
            struct SElementMap
            {
                std::unique_ptr<const IElementMapper> stickLeftX = nullptr;
                std::unique_ptr<const IElementMapper> stickLeftY = nullptr;
                std::unique_ptr<const IElementMapper> stickRightX = nullptr;
                std::unique_ptr<const IElementMapper> stickRightY = nullptr;
                std::unique_ptr<const IElementMapper> dpadUp = nullptr;
                std::unique_ptr<const IElementMapper> dpadDown = nullptr;
                std::unique_ptr<const IElementMapper> dpadLeft = nullptr;
                std::unique_ptr<const IElementMapper> dpadRight = nullptr;
                std::unique_ptr<const IElementMapper> triggerLT = nullptr;
                std::unique_ptr<const IElementMapper> triggerRT = nullptr;
                std::unique_ptr<const IElementMapper> buttonA = nullptr;
                std::unique_ptr<const IElementMapper> buttonB = nullptr;
                std::unique_ptr<const IElementMapper> buttonX = nullptr;
                std::unique_ptr<const IElementMapper> buttonY = nullptr;
                std::unique_ptr<const IElementMapper> buttonLB = nullptr;
                std::unique_ptr<const IElementMapper> buttonRB = nullptr;
                std::unique_ptr<const IElementMapper> buttonBack = nullptr;
                std::unique_ptr<const IElementMapper> buttonStart = nullptr;
                std::unique_ptr<const IElementMapper> buttonLS = nullptr;
                std::unique_ptr<const IElementMapper> buttonRS = nullptr;
            };

            /// Dual representation of a controller element map. Intended for internal use only.
            /// In one representation the elements all have names for element-specific access.
            /// In the other, all the elements are collapsed into an array for easy iteration.
            union UElementMap
            {
                SElementMap named;
                std::unique_ptr<const IElementMapper> all[sizeof(SElementMap) / sizeof(std::unique_ptr<const IElementMapper>)];

                static_assert(sizeof(named) == sizeof(all), "Element map field mismatch.");

                /// Default constructor.
                /// Delegates to the underlying element map.
                inline UElementMap(void) : named()
                {
                    // Nothing to do here.
                }

                /// Copy constructor.
                /// Clones each element mapper that is present in the underlying element map.
                UElementMap(const UElementMap& other);

                /// Move constructor.
                /// Consumes the underlying element map from the other object.
                inline UElementMap(UElementMap&& other) : named(std::move(other.named))
                {
                    // Nothing to do here.
                }

                /// Conversion constructor.
                /// Consumes an element map and promotes it to this dual view.
                inline UElementMap(SElementMap&& named) : named(std::move(named))
                {
                    // Nothing to do here.
                }

                /// Default destructor.
                /// Delegates to the underlying element map.
                inline ~UElementMap(void)
                {
                    named.~SElementMap();
                }

                /// Copy assignment operator.
                /// Clones each underlying element from the other object.
                UElementMap& operator=(const UElementMap& other);

                /// Move assignment operator.
                /// Consumes each underlying element from the other object.
                UElementMap& operator=(UElementMap&& other);
            };

        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// All controller element mappers.
            const UElementMap elements;

            /// Capabilities of the controller described by the element mappers in aggregate.
            /// Initialization of this member depends on prior initialization of #elements so it must come after.
            const SCapabilities capabilities;

            /// Name of this mapper.
            const std::wstring_view name;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor.
            /// Requires a name and, for each controller element, a unique element mapper which becomes owned by this object.
            /// For controller elements that are not used, `nullptr` may be set instead.
            Mapper(const std::wstring_view name, SElementMap&& elements);

            /// Initialization constructor.
            /// Does not require or register a name for this mapper. This version is primarily useful for testing.
            /// Requires that a unique mapper be specified for each controller element, which in turn becomes owned by this object.
            /// For controller elements that are not used, `nullptr` may be set instead.
            Mapper(SElementMap&& elements);

            /// Default destructor.
            /// In general, mapper objects should not be destroyed once created.
            /// However, tests may create mappers as temporaries that end up being destroyed.
            ~Mapper(void);


            // -------- CLASS METHODS -------------------------------------- //

            /// Dumps information about all registered mappers.
            static void DumpRegisteredMappers(void);

            /// Retrieves and returns a pointer to the mapper object whose name is specified.
            /// Mapper objects are created and managed internally, so this operation does not dynamically allocate or deallocate memory, nor should the caller attempt to free the returned pointer.
            /// @param [in] mapperName Name of the desired mapper. Supported built-in values are defined in "MapperDefinitions.cpp" as mapper instances, but more could be built and registered at runtime.
            /// @return Pointer to the mapper of specified name, or `nullptr` if said mapper is unavailable.
            static const Mapper* GetByName(std::wstring_view mapperName);

            /// Retrieves and returns a pointer to the mapper object whose type is read from the configuration file for the specified controller identifier.
            /// If no mapper specified there, then the default mapper type is used instead.
            /// @param [in] controllerIdentifier Identifier of the controller for which a mapper is requested.
            static const Mapper* GetConfigured(TControllerIdentifier controllerIdentifier);

            /// Retrieves and returns a pointer to the default mapper object.
            /// @return Pointer to the default mapper object, or `nullptr` if there is no default.
            static inline const Mapper* GetDefault(void)
            {
                return GetByName(L"");
            }

            /// Retrieves and returns a pointer to a mapper object that does nothing and affects no controller elements.
            /// Can be used as a fall-back in the event of an error. Always returns a valid address.
            /// @return Pointer to the null mapper object.
            static const Mapper* GetNull(void);

            /// Checks if a mapper of the specified name is known and registered.
            /// @param [in] mapperName Name of the mapper to check.
            /// @return `true` if it is registered, `false` otherwise.
            static inline bool IsMapperNameKnown(std::wstring_view mapperName)
            {
                return (nullptr != GetByName(mapperName));
            }


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Returns a copy of this mapper's element map.
            /// Useful for dynamically generating new mappers using this mapper as a template.
            /// @return Copy of this mapper's element map.
            inline UElementMap CloneElementMap(void) const
            {
                return elements;
            }

            /// Returns a read-only reference to this mapper's element map.
            /// Primarily useful for tests.
            /// @return Read-only reference to this mapper's element map.
            inline const UElementMap& ElementMap(void) const
            {
                return elements;
            }

            /// Retrieves and returns the capabilities of the virtual controller layout implemented by the mapper.
            /// Controller capabilities act as metadata that are used internally and can be presented to applications.
            /// @return Capabilities of the virtual controller.
            inline SCapabilities GetCapabilities(void) const
            {
                return capabilities;
            }

            /// Retrieves and returns the name of this mapper.
            /// @return Mapper name.
            inline std::wstring_view GetName(void) const
            {
                return name;
            }

            /// Initializes and fills in the specified virtual controller state data structure object using the specified XInput controller state information.
            /// Does not apply any properties configured by the application, such as deadzone and range. All values produced use standard XInput settings.
            /// @param [in] controllerIdentifier Identifier of the controller making this request. Some element mappers use this information.
            /// @param [out] controllerState Controller state object to be filled.
            /// @param [in] xinputState XInput controller state from which to read.
            void MapXInputState(TControllerIdentifier controllerIdentifier, SState& controllerState, XINPUT_GAMEPAD xinputState) const;
        };
    }
}
