/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperBuilder.h
 *   Declaration of functionality for building new mapper objects piece-wise
 *   at runtime.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"

#include <map>
#include <memory>
#include <optional>
#include <string_view>


namespace Xidi
{
    namespace Controller
    {
        /// Encapsulates all functionality for managing a set of partially-built mappers and constructing them into full mapper objects.
        class MapperBuilder
        {
        public:
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// Holds a description about how to build a single mapper object.
            struct SBlueprint
            {
                /// Name of the mapper that will be used as a template.
                /// Templates are useful for building new mappers based on other mappers.
                /// If no template is specified then the mapper is being built completely from scratch.
                /// A mapper with this as its name is resolved at mapper build time, not at name setting time.
                std::wstring_view templateName;

                /// Holds changes to be applied to the template when the mapper is being built.
                /// For mappers being built from scratch without a template, holds all of the controller element mappers.
                Mapper::UElementMap deltaFromTemplate;

                /// Flag for specifying if this mapper is currently in the process of being built.
                /// Used to detect dependency cycles due to mappers specifying each other as templates.
                bool buildInProgress;

                /// Flag for specifying if the mapper described by this object has already been built successfully.
                /// Once the mapper described by this object is built, this object is no longer useful.
                bool buildCompleted;
            };


        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Holds all known mapper blueprints.
            std::map<std::wstring_view, SBlueprint> blueprints;


        public:
            // -------- CLASS METHODS -------------------------------------- //

            /// Determines if the specified controller element string is valid and recognized as identifying an XInput controller element.
            /// @param [in] element String to be checked.
            /// @return `true` if the input string is valid, `false` otherwise.
            static bool IsControllerElementStringValid(std::wstring_view element);


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Attempts to build mapper objects based on all of the blueprints known to this mapper builder object.
            /// Once a build attempt is made on a blueprint, that blueprint can no longer be modified.
            /// @return `true` if successful in building all of them, `false` otherwise.
            bool Build(void);

            /// Creates a new mapper blueprint object with the specified mapper name.
            /// This method will fail if a mapper or mapper blueprint already exists with the specified name.
            /// @param [in] mapperName Name that identifies the mapper to be described by the blueprint.
            /// @return `true` if successful, `false` otherwise.
            bool CreateBlueprint(std::wstring_view mapperName);

            /// Determines if the specified mapper name already exists as a blueprint within this object.
            /// @param [in] mapperName Name that identifies the mapper described by a possibly-existing blueprint.
            /// @return `true` if the mapper name already exists, `false` otherwise.
            bool DoesBlueprintNameExist(std::wstring_view mapperName) const;

            /// Retrieves and returns a read-only pointer to the element map associated with the blueprint for the mapper of the specified name.
            /// Primarily useful for testing.
            /// @param [in] mapperName Name that identifies the mapper described by a possibly-existing blueprint.
            /// @return Pointer to the blueprint's element map if the blueprint exists, or `nullptr` otherwise.
            const Mapper::UElementMap* GetBlueprintElementMap(std::wstring_view mapperName) const;

            /// Retrieves and returns the template name associated with the blueprint for the mapper of the specified name.
            /// @param [in] mapperName Name that identifies the mapper described by a possibly-existing blueprint.
            /// @return Template name associated with the blueprint if the blueprint exists.
            std::optional<std::wstring_view> GetBlueprintTemplate(std::wstring_view mapperName) const;

            /// Sets a specific element mapper to be applied as a delta to the template when this object is built into a mapper.
            /// This method will fail if the mapper name does not identify an existing blueprint or if the element string cannot be mapped to a valid XInput controller element.
            /// Valid controller element strings are field names for #SElementMap with the first letter capitalized.
            /// @param [in] mapperName Name that identifies the mapper whose element is being set.
            /// @param [in] element String that identifies the XInput controller element. Must be null-terminated.
            /// @param [in] elementMapper Element mapper to use, which becomes owned by this object.
            /// @return `true` if successful, `false` otherwise.
            bool SetBlueprintElementMapper(std::wstring_view mapperName, std::wstring_view element, std::unique_ptr<IElementMapper>&& elementMapper);

            /// Sets the name of the mapper that will act as a template for the mapper being built.
            /// Template names are resolved when attempting to construct a mapper object, so it is not necessary for the template name to identify an existing mapper or mapper blueprint.
            /// This method will fail if the mapper name does not identify an existing blueprint.
            /// @param [in] mapperName Name that identifies the mapper whose template is being set.
            /// @param [in] newTemplateName New template name.
            /// @return `true` if successful, `false` otherwise.
            bool SetBlueprintTemplate(std::wstring_view mapperName, std::wstring_view newTemplateName);
        };
    }
}
