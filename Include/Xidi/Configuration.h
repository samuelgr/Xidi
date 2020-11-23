/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Configuration.h
 *   Declaration of configuration file functionality.
 *****************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>


namespace Xidi
{
    namespace Configuration
    {
        // -------- CONSTANTS ------------------------------------------ //

        /// Section name for all settings that appear at global scope (i.e. outside of a section).
        inline constexpr wchar_t kSectionNameGlobal[] = L"";


        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Enumerates the possible results of reading a configuration file.
        enum class EFileReadResult
        {
            InvalidResult = -1,                                             ///< No attempt was made to read the configuration file. Used during initializion.
            Success,                                                        ///< Configuration file was read successfully.
            FileNotFound,                                                   ///< Configuration file does not exist.
            Malformed,                                                      ///< Configuration file is malformed.
        };

        /// Enumerates all supported actions for configuration sections.
        /// Used when checking with a subclass for guidance on what to do when a particular named section is encountered.
        enum class ESectionAction
        {
            Error,                                                          ///< Section name is not supported.
            Read,                                                           ///< Section name is supported and interesting, so the section will be read.
            Skip,                                                           ///< Section name is supported but uninteresting, so the whole section should be skipped.
        };

        /// Enumerates all supported types for configuration values.
        /// Used when checking with a subclass for guidance on whether a section/name pair is supported and, if so, how to parse the value.
        enum class EValueType
        {
            Error,                                                          ///< Combination of section and name pair is not supported.
            Integer,                                                        ///< Combination of section and name pair is supported; value is a single integer.
            Boolean,                                                        ///< Combination of section and name pair is supported; value is a single Boolean.
            String,                                                         ///< Combination of section and name pair is supported; value is a single string.
            IntegerMultiValue,                                              ///< Combination of section and name pair is supported; value is integer and multiple values are allowed.
            BooleanMultiValue,                                              ///< Combination of section and name pair is supported; value is Boolean and multiple values are allowed.
            StringMultiValue,                                               ///< Combination of section and name pair is supported; value is string and multiple values are allowed.
        };

        /// Underlying type used for storing integer-typed values.
        typedef int64_t TIntegerValue;

        /// Underlying type used for storing Boolean-valued types.
        typedef bool TBooleanValue;

        /// Underlying type used for storing string-valued types.
        typedef std::wstring TStringValue;

        /// Fourth-level object used to represent a single configuration value for a particular configuration setting.
        class Value
        {
        private:
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// View type used for retrieving and returning integer-typed values.
            typedef TIntegerValue TIntegerView;

            /// View type used for retrieving and returning integer-typed values.
            typedef TBooleanValue TBooleanView;

            /// View type used for retrieving and returning string-typed values.
            typedef std::wstring_view TStringView;


            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Indicates the value type.
            EValueType type;

            /// Holds the value itself.
            union
            {
                TIntegerValue intValue;
                TBooleanValue boolValue;
                TStringValue stringValue;
            };


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor. Creates an integer-typed value.
            inline Value(const TIntegerValue& value) : type(EValueType::Integer), intValue(value)
            {
                // Nothing to do here.
            }

            /// Initialization constructor. Creates a Boolean-typed value.
            inline Value(const TBooleanValue& value) : type(EValueType::Boolean), boolValue(value)
            {
                // Nothing to do here.
            }

            /// Initialization constructor. Creates a string-typed value.
            inline Value(const TStringValue& value) : type(EValueType::String), stringValue(value)
            {
                // Nothing to do here.
            }

            /// Default destructor.
            inline ~Value(void)
            {
                switch (type)
                {
                case EValueType::Integer:
                    intValue.~TIntegerValue();
                    break;

                case EValueType::Boolean:
                    boolValue.~TBooleanValue();
                    break;

                case EValueType::String:
                    stringValue.~TStringValue();
                    break;

                default:
                    break;
                }
            }

            /// Copy constructor.
            inline Value(const Value& other) : type(other.type)
            {
                switch (other.type)
                {
                case EValueType::Integer:
                    new (&intValue) TIntegerValue(other.intValue);
                    break;

                case EValueType::Boolean:
                    new (&boolValue) TBooleanValue(other.boolValue);
                    break;

                case EValueType::String:
                    new (&stringValue) TStringValue(other.stringValue);
                    break;

                default:
                    break;
                }
            }

            /// Move constructor.
            inline Value(Value&& other) : type(std::move(other.type))
            {
                switch (other.type)
                {
                case EValueType::Integer:
                    new (&intValue) TIntegerValue(std::move(other.intValue));
                    break;

                case EValueType::Boolean:
                    new (&boolValue) TBooleanValue(std::move(other.boolValue));
                    break;

                case EValueType::String:
                    new (&stringValue) TStringValue(std::move(other.stringValue));
                    break;

                default:
                    break;
                }
            }


            // -------- OPERATORS ---------------------------------------------- //

            /// Allows less-than comparison to support many standard containers that use this type of comparison for sorting.
            /// @param [in] rhs Right-hand side of the binary operator.
            /// @return `true` if this object (lhs) is less than the other object (rhs), `false` otherwise.
            inline bool operator<(const Value& rhs) const
            {
                if (type < rhs.type)
                    return true;
                else if (type > rhs.type)
                    return false;
                else
                {
                    switch (type)
                    {
                    case EValueType::Integer:
                        return (intValue < rhs.intValue);

                    case EValueType::Boolean:
                        return (boolValue < rhs.boolValue);

                    case EValueType::String:
                        return (stringValue < rhs.stringValue);

                    default:
                        return false;
                    }
                }
            }


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Retrieves and returns the type of the stored value.
            /// @return Type of the stored value.
            inline EValueType GetType(void) const
            {
                return type;
            }

            /// Retrieves and returns an immutable reference to the stored value as an integer.
            /// Does not ensure the type of value is actually integer.
            /// @return Stored value.
            inline const TIntegerView GetIntegerValue(void) const
            {
                return intValue;
            }

            /// Retrieves and returns an immutable reference to the stored value as a Boolean.
            /// Does not ensure the type of value is actually Boolean.
            /// @return Stored value.
            inline const TBooleanView GetBooleanValue(void) const
            {
                return boolValue;
            }

            /// Retrieves and returns an immutable reference to the stored value as a string.
            /// Does not ensure the type of value is actually string.
            /// @return Stored value.
            inline const TStringView GetStringValue(void) const
            {
                return stringValue;
            }

        };

        /// Third-level object used to represent a single configuration setting within one section of a configuration file.
        class Name
        {
        private:
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// Alias for the underlying data structure used to store per-setting configuration values.
            typedef std::set<Value> TValues;


            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Holds all values for each configuration setting, one element per value.
            TValues values;


        public:
            // -------- INSTANCE METHODS ----------------------------------- //

            /// Allows read-only access to the first stored value.
            /// Useful for single-valued settings.
            /// @return First stored value.
            inline const Value& FirstValue(void) const
            {
                return *(values.begin());
            }

            /// Stores a new value for the configuration setting represented by this object.
            /// Will fail if the value already exists.
            /// @tparam ValueType Type of value to insert.
            /// @param [in] value Value to insert.
            /// @return `true` on success, `false` on failure.
            template <typename ValueType> bool Insert(const ValueType& value)
            {
                return values.emplace(value).second;
            }

            /// Retrieves the number of values present for the configuration setting represented by this object.
            /// @return Number of values present.
            inline size_t ValueCount(void) const
            {
                return values.size();
            }

            /// Allows read-only access to all values.
            /// Useful for iterating.
            /// @return Container of all values.
            inline const TValues& Values(void) const
            {
                return values;
            }
        };

        /// Second-level object used to represent an entire section of a configuration file.
        class Section
        {
        private:
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// Alias for the underlying data structure used to store per-section configuration settings.
            typedef std::map<std::wstring, Name, std::less<>> TNames;


            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Holds configuration data within each section, one element per configuration setting.
            TNames names;


        public:
            // -------- OPERATORS ------------------------------------------ //

            /// Allows read-only access to individual configuration settings by name, without bounds checking.
            /// @param [in] name Name of the configuration setting to retrieve.
            /// @return Reference to the desired configuration setting.
            inline const Name& operator[](std::wstring_view name) const
            {
                return names.find(name)->second;
            }


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Stores a new value for the specified configuration setting in the section represented by this object.
            /// Will fail if the value already exists.
            /// @tparam ValueType Type of value to insert.
            /// @param [in] name Name of the configuration setting into which to insert the value.
            /// @param [in] value Value to insert.
            /// @return `true` on success, `false` on failure.
            template <typename ValueType> bool Insert(std::wstring_view name, const ValueType& value)
            {
                auto nameIterator = names.find(name);

                if (names.end() == nameIterator)
                {
                    names.emplace(name, Name());
                    nameIterator = names.find(name);
                }

                return nameIterator->second.Insert(value);
            }

            /// Retrieves the number of configuration settings present for the section represented by this object.
            /// @return Number of configuration settings present.
            inline size_t NameCount(void) const
            {
                return names.size();
            }

            /// Determines if a configuration setting of the specified name exists in the section represented by this object.
            /// @param [in] name Name of the configuration setting to check.
            /// @return `true` if the setting exists, `false` otherwise.
            inline bool NameExists(std::wstring_view name) const
            {
                return (0 != names.count(name));
            }

            /// Allows read-only access to all configuration settings.
            /// Useful for iterating.
            /// @return Container of all configuration settings.
            inline const TNames& Names(void) const
            {
                return names;
            }
        };

        /// Top-level object used to represent all configuration data read from a configuration file.
        class ConfigurationData
        {
        private:
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// Alias for the underlying data structure used to store top-level configuration section data.
            typedef std::map<std::wstring, Section, std::less<>> TSections;


            /// Holds an individual section and name pair.
            /// Used when responding to queries for all settings of a given name across all sections.
            struct SSectionNamePair
            {
                std::wstring_view section;                                     ///< Name of the section that holds the identified configuration setting.
                const Name& name;                                           ///< Reference to the object that holds all values for the identified configuration setting.

                /// Initialization constructor. Initializes both references.
                inline constexpr SSectionNamePair(std::wstring_view section, const Name& name) : section(section), name(name)
                {
                    // Nothing to do here.
                }
            };

            /// Alias for the data structure used to respond to queries for all settings of a given name across all sections.
            typedef std::list<SSectionNamePair> TSectionNamePairList;


            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Holds configuration data at the level of entire sections, one element per section.
            TSections sections;


        public:
            // -------- OPERATORS ------------------------------------------ //

            /// Allows read-only access to individual sections by name, without bounds checking.
            /// @param [in] section Name of the section to retrieve.
            /// @return Reference to the desired section.
            inline const Section& operator[](std::wstring_view section) const
            {
                return sections.find(section)->second;
            }


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Clears the contents of this object.
            /// After clearing, all references to its contents (such as via data structures returned by querying it) are invalid.
            inline void Clear(void)
            {
                sections.clear();
            }

            /// Stores a new value for the specified configuration setting in the specified section.
            /// Will fail if the value already exists.
            /// @tparam ValueType Type of value to insert.
            /// @param [in] section Section into which to insert the configuration setting.
            /// @param [in] name Name of the configuration setting into which to insert the value.
            /// @param [in] value Value to insert.
            /// @return `true` on success, `false` on failure.
            template <typename ValueType> bool Insert(std::wstring_view section, std::wstring_view name, const ValueType& value)
            {
                auto sectionIterator = sections.find(section);

                if (sections.end() == sectionIterator)
                {
                    sections.emplace(section, Section());
                    sectionIterator = sections.find(section);
                }

                return sectionIterator->second.Insert(name, value);
            }

            /// Retrieves the number of sections present in the configuration represented by this object.
            /// @return Number of configuration settings present.
            inline size_t SectionCount(void) const
            {
                return sections.size();
            }

            /// Determines if a section of the specified name exists in the configuration represented by this object.
            /// @param [in] section Section name to check.
            /// @return `true` if the setting exists, `false` otherwise.
            inline bool SectionExists(std::wstring_view section) const
            {
                return (0 != sections.count(section));
            }

            /// Determines if a configuration setting of the specified name exists in the specified section.
            /// @param [in] section Section name to check.
            /// @param [in] name Name of the configuration setting to check.
            /// @return `true` if the setting exists, `false` otherwise.
            inline bool SectionNamePairExists(std::wstring_view section, std::wstring_view name) const
            {
                auto sectionIterator = sections.find(section);
                if (sections.end() == sectionIterator)
                    return false;

                return sectionIterator->second.NameExists(name);
            }

            /// Allows read-only access to all sections.
            /// Useful for iterating.
            /// @return Container of all sections.
            inline const TSections& Sections(void) const
            {
                return sections;
            }

            /// Searches all sections in the configuration for settings identified by the specified name.
            /// For each, identifies both the section (by name) and the configuration setting (by the object that holds its values).
            /// Places all such pairs into a container and returns the container.
            /// If there are no matches, returns an empty container.
            /// @param [in] name Name of the configuration setting for which to search.
            /// @return Container holding the results.
            inline std::unique_ptr<TSectionNamePairList> SectionsContaining(std::wstring_view name) const
            {
                std::unique_ptr<TSectionNamePairList> sectionsWithName = std::make_unique<TSectionNamePairList>();

                for (auto& section : sections)
                {
                    if (section.second.NameExists(name))
                        sectionsWithName->emplace_back(section.first, section.second[name]);
                }

                return sectionsWithName;
            }
        };

        /// Interface for reading and parsing INI-formatted configuration files.
        /// Name-and-value pairs (of the format "name = value") are namespaced by sections (of the format "[section name]").
        /// Provides basic configuration file reading and parsing functionality, but leaves managing and error-checking configuration values to subclasses.
        class ConfigurationFileReader
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Holds the error message that describes the error that arose during the last unsuccessful attempt at reading a configuration file.
            std::wstring readErrorMessage;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Default destructor.
            virtual ~ConfigurationFileReader(void) = default;


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Retrieves and returns the error message that arose during the last unsuccessful attempt at reading a configuration file.
            /// The error message is valid if #ReadConfigurationFile returns anything other than success.
            /// @return Error message from last unsuccessful read attempt.
            inline std::wstring_view GetReadErrorMessage(void)
            {
                return readErrorMessage;
            }

            /// Reads and parses a configuration file, storing the settings in the supplied configuration object.
            /// Intended to be invoked externally. Subclasses should not override this method.
            /// @param [in] configFileName Name of the configuration file to read.
            /// @param [out] configToFill Configuration object to fill with configuration data (contents are only valid if this method succeeds).
            /// @return Indicator of the result of the operation.
            EFileReadResult ReadConfigurationFile(std::wstring_view configFileName, ConfigurationData& configToFill);


        private:
            // -------- ABSTRACT INSTANCE METHODS -------------------------- //

            /// Specifies the action to take when a given section is encountered in a configuration file.
            /// These are the names that typically appear in [square brackets].
            /// Invoked while reading from a configuration file.
            /// Subclasses should override this method.
            /// For example, if the particular section name is not within the list of supported configuration namespaces, subclasses can flag an error.
            /// @param [in] section Name of the section, as read from the configuration file.
            /// @return Action to take with the section.
            virtual ESectionAction ActionForSection(std::wstring_view section) = 0;

            /// Invoked to allow the subclass to error-check the specified integer-typed configuration setting, identified by enclosing section name and by configuration setting name.
            /// @param [in] section Name of the enclosing section, as read from the configuration file.
            /// @param [in] name Name of the configuration setting, as read from the configuration file.
            /// @param [in] value Value of the configuration setting, as read and parsed from the configuration file.
            /// @return `true` if the submitted value was acceptable (according to whatever arbitrary characteristics the subclass wishes), `false` otherwise.
            virtual bool CheckValue(std::wstring_view section, std::wstring_view name, const TIntegerValue& value) = 0;

            /// Invoked to allow the subclass to error-check the specified Boolean-typed configuration setting, identified by enclosing section name and by configuration setting name.
            /// @param [in] section Name of the enclosing section, as read from the configuration file.
            /// @param [in] name Name of the configuration setting, as read from the configuration file.
            /// @param [in] value Value of the configuration setting, as read and parsed from the configuration file.
            /// @return `true` if the submitted value was acceptable (according to whatever arbitrary characteristics the subclass wishes), `false` otherwise.
            virtual bool CheckValue(std::wstring_view section, std::wstring_view name, const TBooleanValue& value) = 0;

            /// Invoked to allow the subclass to error-check specified string-typed configuration setting, identified by enclosing section name and by configuration setting name.
            /// @param [in] section Name of the enclosing section, as read from the configuration file.
            /// @param [in] name Name of the configuration setting, as read from the configuration file.
            /// @param [in] value Value of the configuration setting, as read and parsed from the configuration file.
            /// @return `true` if the submitted value was acceptable (according to whatever arbitrary characteristics the subclass wishes), `false` otherwise.
            virtual bool CheckValue(std::wstring_view section, std::wstring_view name, const TStringValue& value) = 0;

            /// Specifies the type of the value for the given configuration setting.
            /// In lines that are of the form "name = value" parameters identify both the enclosing section and the name part.
            /// Subclasses should override this method.
            /// For example, if the value is expected to be an integer, subclasses should indicate this so the value is parsed and submitted correctly.
            /// @param [in] section Name of the enclosing section, as read from the configuration file.
            /// @param [in] name Name of the configuration setting (the left part of the example line given above).
            /// @return Type to associate with the value (the right part of the example line given above), which can be an error if the particular configuration setting is not supported.
            virtual EValueType TypeForValue(std::wstring_view section, std::wstring_view name) = 0;


            // -------- CONCRETE INSTANCE METHODS -------------------------- //

            /// Invoked at the start of a configuration file read operation.
            /// Subclasses are given the opportunity to initialize or reset any stored state, as needed.
            /// Overriding this method is optional, as a default implementation exists that does nothing.
            virtual void PrepareForRead(void);
        };

        /// Convenience wrapper object that combines a reader with a configuration data object and presents both with a unified interface.
        class Configuration
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Reader object used to dictate how a configuration file is read.
            std::unique_ptr<ConfigurationFileReader> reader;

            /// Configuration data object used to hold configuration data read from the configuration file.
            ConfigurationData configData;

            /// Holds the result of the last attempt at reading a configuration file.
            EFileReadResult fileReadResult;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor. Requires a reader at construction time.
            inline Configuration(std::unique_ptr<ConfigurationFileReader> reader) : reader(std::move(reader)), configData(), fileReadResult(EFileReadResult::InvalidResult)
            {
                // Nothing to do here.
            }

            /// Copy constructor. Should never be invoked.
            Configuration(const Configuration& other) = delete;

            // -------- INSTANCE METHODS ----------------------------------- //

            /// Retrieves and returns a reference to the object that holds all of Xidi's configuration settings.
            /// @return Configuration settings object.
            inline const ConfigurationData& GetData(void) const
            {
                return configData;
            }

            /// Determines if the contents of the object that holds all of Xidi's configuration settings are valid.
            /// If a previous attempt to read the Xidi configuration file failed,
            /// @return `true` if the contents are valid, `false` otherwise.
            inline EFileReadResult GetFileReadResult(void) const
            {
                return fileReadResult;
            }

            /// Retrieves and returns the error message that arose during the last unsuccessful attempt at reading a configuration file.
            /// The error message is valid as long as #GetFileReadResult returns anything other than success.
            /// @return Error message from last unsuccessful read attempt.
            inline std::wstring_view GetReadErrorMessage(void) const
            {
                return reader->GetReadErrorMessage();
            }

            /// Determines if the configuration data object contains valid data (i.e. the configuration file was read and parsed successfully).
            /// @return `true` if it contains valid data, `false` if not.
            inline bool IsDataValid(void) const
            {
                return (EFileReadResult::Success == GetFileReadResult());
            }

            /// Reads and parses a configuration file, storing the settings in this object.
            /// After this method returns, use #GetFileReadResult and #GetData to retrieve configuration settings.
            /// In the event of a read error, #GetReadErrorMessage can be used to obtain a string describing the read error that occurred.
            /// @param [in] configFileName Name of the configuration file to read.
            inline void ReadConfigurationFile(std::wstring_view configFileName)
            {
                fileReadResult = reader->ReadConfigurationFile(configFileName, configData);
            }
        };

        /// Type alias for a suggested format for storing the supported layout of a section within a configuration file.
        /// Useful for pre-determining what is allowed to appear within one section of a configuration file.
        typedef std::map<std::wstring_view, EValueType, std::less<>> TConfigurationFileSectionLayout;

        /// Type alias for a suggested format for storing the supported layout of a configuration file.
        /// Useful for pre-determining what is allowed to appear within a configuration file.
        typedef std::map<std::wstring_view, TConfigurationFileSectionLayout, std::less<>> TConfigurationFileLayout;
    }
}


// -------- MACROS --------------------------------------------------------- //

/// Convenience wrapper around initializer list syntax for defining a configuration file section in a layout object.
/// Specify a section name followed by a series of setting name and value type pairs.
#define ConfigurationFileLayoutSection(section, ...)                    { (section), {__VA_ARGS__} }

/// Convenience wrapper around initializer list syntax for defining a setting name and value type pair.
/// Intended for use within the initializer for a configuration file section layout.
#define ConfigurationFileLayoutNameAndValueType(name, valueType)        { (name), (valueType) }
