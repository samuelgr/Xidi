/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file Configuration.h
 *   Declaration of configuration file functionality.
 **************************************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

/// Convenience wrapper around initializer list syntax for defining a configuration file section in
/// a layout object. Specify a section name followed by a series of setting name and value type
/// pairs.
#define ConfigurationFileLayoutSection(section, ...)                                               \
  {                                                                                                \
    (section),                                                                                     \
    {                                                                                              \
      __VA_ARGS__                                                                                  \
    }                                                                                              \
  }

/// Convenience wrapper around initializer list syntax for defining a setting name and value type
/// pair. Intended for use within the initializer for a configuration file section layout.
#define ConfigurationFileLayoutNameAndValueType(name, valueType)                                   \
  {                                                                                                \
    (name), (valueType)                                                                            \
  }

namespace Xidi
{
  namespace Configuration
  {
    /// Section name for all settings that appear at global scope (i.e. outside of a section).
    inline constexpr wchar_t kSectionNameGlobal[] = L"";

    /// Enumerates possible directives that can be issued in response to a query on how to
    /// process a section or a name/value pair encountered in a configuration file.
    enum class EAction
    {
      /// Flag an error. For sections, this means the remainder of the section is skipped.
      Error,

      /// Continue processing. For sections this means the name/value pairs within will be
      /// read. For name/value pairs this means the pair will be inserted into the
      /// configuration data structure.
      Process,

      /// Skip. For sections this means to ignore all the name/value pairs within. For
      /// name/value pairs this means to do nothing.
      Skip,
    };

    /// Enumerates all supported types for configuration values.
    /// Used when checking with a subclass for guidance on whether a section/name pair is
    /// supported and, if so, how to parse the value.
    enum class EValueType
    {
      /// Combination of section and name pair is not supported.
      Error,

      /// Combination of section and name pair is supported; value is a single integer.
      Integer,

      /// Combination of section and name pair is supported; value is a single Boolean.
      Boolean,

      /// Combination of section and name pair is supported; value is a single string.
      String,

      /// Combination of section and name pair is supported; value is integer and multiple
      /// values are allowed.
      IntegerMultiValue,

      /// Combination of section and name pair is supported; value is Boolean and multiple
      /// values are allowed.
      BooleanMultiValue,

      /// Combination of section and name pair is supported; value is string and multiple
      /// values are allowed.
      StringMultiValue,
    };

    /// Underlying type used for storing integer-typed values.
    using TIntegerValue = int64_t;

    /// Underlying type used for storing Boolean-valued types.
    using TBooleanValue = bool;

    /// Underlying type used for storing string-valued types.
    using TStringValue = std::wstring;

    /// View type used for retrieving and returning integer-typed values.
    using TIntegerView = TIntegerValue;

    /// View type used for retrieving and returning integer-typed values.
    using TBooleanView = TBooleanValue;

    /// View type used for retrieving and returning string-typed values.
    using TStringView = std::wstring_view;

    /// Fourth-level object used to represent a single configuration value for a particular
    /// configuration setting.
    class Value
    {
    public:

      /// Creates an integer-typed value by copying it.
      inline Value(const TIntegerValue& value) : type(EValueType::Integer), intValue(value) {}

      /// Creates an integer-typed value by moving it.
      inline Value(TIntegerValue&& value) : type(EValueType::Integer), intValue(std::move(value)) {}

      /// Creates a Boolean-typed value by copying it.
      inline Value(const TBooleanValue& value) : type(EValueType::Boolean), boolValue(value) {}

      /// Creates a Boolean-typed value by moving it.
      inline Value(TBooleanValue&& value) : type(EValueType::Boolean), boolValue(std::move(value))
      {}

      /// Creates a string-typed value by copying it.
      inline Value(const TStringValue& value) : type(EValueType::String), stringValue(value) {}

      /// Creates a string-typed value by moving it.
      inline Value(TStringValue&& value) : type(EValueType::String), stringValue(std::move(value))
      {}

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

      inline bool operator<(const Value& rhs) const
      {
        if (type < rhs.type)
          return true;
        else if (type > rhs.type)
          return false;

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

      inline bool operator==(const Value& rhs) const
      {
        if (type != rhs.type) return false;

        switch (type)
        {
          case EValueType::Integer:
            return (intValue == rhs.intValue);

          case EValueType::Boolean:
            return (boolValue == rhs.boolValue);

          case EValueType::String:
            return (stringValue == rhs.stringValue);

          default:
            return false;
        }
      }

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

    private:

      /// Indicates the value type.
      EValueType type;

      /// Holds the value itself.
      union
      {
        TIntegerValue intValue;
        TBooleanValue boolValue;
        TStringValue stringValue;
      };
    };

    /// Third-level object used to represent a single configuration setting within one section of a
    /// configuration file.
    class Name
    {
    public:

      /// Alias for the underlying data structure used to store per-setting configuration values.
      using TValues = std::set<Value>;

      /// Inserts an initial value by copying it. All objects are required to contain at least one
      /// value.
      template <typename ValueType> inline Name(const ValueType& firstValue) : values()
      {
        Insert(firstValue);
      }

      /// Inserts an initial value by moving it. All objects are required to contain at least one
      /// value.
      template <typename ValueType> inline Name(ValueType&& firstValue) : values()
      {
        Insert(std::move(firstValue));
      }

      /// Allows read-only access to the first stored value, which is guaranteed to exist.
      /// Useful for single-valued settings.
      /// @return First stored value.
      inline const Value& FirstValue(void) const
      {
        return *(values.begin());
      }

      /// Stores a new value for the configuration setting represented by this object by copying the
      /// input parameter. Will fail if the value already exists.
      /// @tparam ValueType Type of value to insert.
      /// @param [in] value Value to insert.
      /// @return `true` on success, `false` on failure.
      template <typename ValueType> bool Insert(const ValueType& value)
      {
        return Insert(ValueType(value));
      }

      /// Stores a new value for the configuration setting represented by this object by moving the
      /// input parameter. Will fail if the value already exists.
      /// @tparam ValueType Type of value to insert.
      /// @param [in] value Value to insert.
      /// @return `true` on success, `false` on failure.
      template <typename ValueType> bool Insert(ValueType&& value)
      {
        return values.emplace(std::move(value)).second;
      }

      /// Retrieves the number of values present for the configuration setting represented by this
      /// object.
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

    private:

      /// Holds all values for each configuration setting, one element per value.
      TValues values;
    };

    /// Second-level object used to represent an entire section of a configuration file.
    class Section
    {
    public:

      /// Alias for the underlying data structure used to store per-section configuration settings.
      using TNames = std::map<std::wstring, Name, std::less<>>;

      /// Allows read-only access to individual configuration settings by name, without bounds
      /// checking.
      /// @param [in] name Name of the configuration setting to retrieve.
      /// @return Reference to the desired configuration setting.
      inline const Name& operator[](std::wstring_view name) const
      {
        return names.find(name)->second;
      }

      /// Convenience wrapper for quickly attempting to obtain a single Boolean-typed configuration
      /// value.
      /// @param [in] name Name of the value for which to search within this section.
      /// @return First value associated with the section and name, if it exists.
      inline std::optional<TBooleanView> GetFirstBooleanValue(std::wstring_view name) const
      {
        if (false == NameExists(name)) return std::nullopt;

        switch ((*this)[name].FirstValue().GetType())
        {
          case EValueType::Boolean:
          case EValueType::BooleanMultiValue:
            break;

          default:
            return std::nullopt;
        }

        return (*this)[name].FirstValue().GetBooleanValue();
      }

      /// Convenience wrapper for quickly attempting to obtain a single Integer-typed configuration
      /// value.
      /// @param [in] name Name of the value for which to search within this section.
      /// @return First value associated with the section and name, if it exists.
      inline std::optional<TIntegerView> GetFirstIntegerValue(std::wstring_view name) const
      {
        if (false == NameExists(name)) return std::nullopt;

        switch ((*this)[name].FirstValue().GetType())
        {
          case EValueType::Integer:
          case EValueType::IntegerMultiValue:
            break;

          default:
            return std::nullopt;
        }

        return (*this)[name].FirstValue().GetIntegerValue();
      }

      /// Convenience wrapper for quickly attempting to obtain a single string-typed configuration
      /// value.
      /// @param [in] name Name of the value for which to search within this section.
      /// @return First value associated with the section and name, if it exists.
      inline std::optional<TStringView> GetFirstStringValue(std::wstring_view name) const
      {
        if (false == NameExists(name)) return std::nullopt;

        switch ((*this)[name].FirstValue().GetType())
        {
          case EValueType::String:
          case EValueType::StringMultiValue:
            break;

          default:
            return std::nullopt;
        }

        return (*this)[name].FirstValue().GetStringValue();
      }

      /// Stores a new value for the specified configuration setting in the section represented by
      /// this object by copying the input parameter. Will fail if the value already exists.
      /// @tparam ValueType Type of value to insert.
      /// @param [in] name Name of the configuration setting into which to insert the value.
      /// @param [in] value Value to insert.
      /// @return `true` on success, `false` on failure.
      template <typename ValueType> inline bool Insert(
          std::wstring_view name, const ValueType& value)
      {
        return Insert(name, ValueType(value));
      }

      /// Stores a new value for the specified configuration setting in the section represented by
      /// this object by moving the input parameter. Will fail if the value already exists.
      /// @tparam ValueType Type of value to insert.
      /// @param [in] name Name of the configuration setting into which to insert the value.
      /// @param [in] value Value to insert.
      /// @return `true` on success, `false` on failure.
      template <typename ValueType> inline bool Insert(std::wstring_view name, ValueType&& value)
      {
        auto nameIterator = names.find(name);

        if (names.end() == nameIterator)
        {
          names.emplace(name, value);
          return true;
        }

        return nameIterator->second.Insert(value);
      }

      /// Retrieves the number of configuration settings present for the section represented by this
      /// object.
      /// @return Number of configuration settings present.
      inline size_t NameCount(void) const
      {
        return names.size();
      }

      /// Determines if a configuration setting of the specified name exists in the section
      /// represented by this object.
      /// @param [in] name Name of the configuration setting to check.
      /// @return `true` if the setting exists, `false` otherwise.
      inline bool NameExists(std::wstring_view name) const
      {
        return names.contains(name);
      }

      /// Allows read-only access to all configuration settings.
      /// Useful for iterating.
      /// @return Container of all configuration settings.
      inline const TNames& Names(void) const
      {
        return names;
      }

    private:

      /// Holds configuration data within each section, one element per configuration setting.
      TNames names;
    };

    /// Top-level object used to represent all configuration data read from a configuration file.
    class ConfigurationData
    {
    public:

      /// Alias for the underlying data structure used to store top-level configuration section
      /// data.
      using TSections = std::map<std::wstring, Section, std::less<>>;

      /// Allows read-only access to individual sections by name, without bounds checking.
      /// @param [in] section Name of the section to retrieve.
      /// @return Reference to the desired section.
      inline const Section& operator[](std::wstring_view section) const
      {
        return sections.find(section)->second;
      }

      /// Stores a new value for the specified configuration setting in the specified section by
      /// copying the input parameter. Will fail if the value already exists.
      /// @tparam ValueType Type of value to insert.
      /// @param [in] section Section into which to insert the configuration setting.
      /// @param [in] name Name of the configuration setting into which to insert the value.
      /// @param [in] value Value to insert.
      /// @return `true` on success, `false` on failure.
      template <typename ValueType> bool Insert(
          std::wstring_view section, std::wstring_view name, const ValueType& value)
      {
        return Insert(ValueType(value));
      }

      /// Stores a new value for the specified configuration setting in the specified section by
      /// moving the input parameter. Will fail if the value already exists.
      /// @tparam ValueType Type of value to insert.
      /// @param [in] section Section into which to insert the configuration setting.
      /// @param [in] name Name of the configuration setting into which to insert the value.
      /// @param [in] value Value to insert.
      /// @return `true` on success, `false` on failure.
      template <typename ValueType> bool Insert(
          std::wstring_view section, std::wstring_view name, ValueType&& value)
      {
        auto sectionIterator = sections.find(section);

        if (sections.end() == sectionIterator)
        {
          sections.emplace(section, Section());
          sectionIterator = sections.find(section);
        }

        return sectionIterator->second.Insert(name, std::move(value));
      }

      /// Convenience wrapper for quickly attempting to obtain a single Boolean-typed configuration
      /// value.
      /// @param [in] section Section name to search for the value.
      /// @param [in] name Name of the value for which to search.
      /// @return First value associated with the section and name, if it exists.
      inline std::optional<TBooleanView> GetFirstBooleanValue(
          std::wstring_view section, std::wstring_view name) const
      {
        if (false == SectionExists(section)) return std::nullopt;

        return (*this)[section].GetFirstBooleanValue(name);
      }

      /// Convenience wrapper for quickly attempting to obtain a single Integer-typed configuration
      /// value.
      /// @param [in] section Section name to search for the value.
      /// @param [in] name Name of the value for which to search.
      /// @return First value associated with the section and name, if it exists.
      inline std::optional<TIntegerView> GetFirstIntegerValue(
          std::wstring_view section, std::wstring_view name) const
      {
        if (false == SectionExists(section)) return std::nullopt;

        return (*this)[section].GetFirstIntegerValue(name);
      }

      /// Convenience wrapper for quickly attempting to obtain a single string-typed configuration
      /// value.
      /// @param [in] section Section name to search for the value.
      /// @param [in] name Name of the value for which to search.
      /// @return First value associated with the section and name, if it exists.
      inline std::optional<TStringView> GetFirstStringValue(
          std::wstring_view section, std::wstring_view name) const
      {
        if (false == SectionExists(section)) return std::nullopt;

        return (*this)[section].GetFirstStringValue(name);
      }

      /// Specifies if one or more errors were encountered while generating the data contained in
      /// this object.
      /// @return `true` if so, `false` if not.
      inline bool HasErrors(void) const
      {
        return hasErrors;
      }

      /// Retrieves the number of sections present in the configuration represented by this object.
      /// @return Number of configuration settings present.
      inline size_t SectionCount(void) const
      {
        return sections.size();
      }

      /// Determines if a section of the specified name exists in the configuration represented by
      /// this object.
      /// @param [in] section Section name to check.
      /// @return `true` if the setting exists, `false` otherwise.
      inline bool SectionExists(std::wstring_view section) const
      {
        return (0 != sections.count(section));
      }

      /// Determines if a configuration setting of the specified name exists in the specified
      /// section.
      /// @param [in] section Section name to check.
      /// @param [in] name Name of the configuration setting to check.
      /// @return `true` if the setting exists, `false` otherwise.
      inline bool SectionNamePairExists(std::wstring_view section, std::wstring_view name) const
      {
        auto sectionIterator = sections.find(section);
        if (sections.end() == sectionIterator) return false;

        return sectionIterator->second.NameExists(name);
      }

      /// Allows read-only access to all sections.
      /// Useful for iterating.
      /// @return Container of all sections.
      inline const TSections& Sections(void) const
      {
        return sections;
      }

      /// Marks this object as having errors associated with the process of inserting data.
      /// For use by whatever function is generating the configuration data to be contained within
      /// this object.
      inline void SetError(void)
      {
        hasErrors = true;
      }

    private:

      /// Holds configuration data at the level of entire sections, one element per section.
      TSections sections;

      /// Specifies if errors were encountered while generating the data contained within this
      /// object.
      bool hasErrors;
    };

    /// Interface for reading and parsing INI-formatted configuration files.
    /// Name-and-value pairs (of the format "name = value") are namespaced by sections (of the
    /// format "[section name]"). Provides basic configuration file reading and parsing
    /// functionality, but leaves managing and error-checking configuration values to subclasses.
    class ConfigurationFileReader
    {
    public:

      virtual ~ConfigurationFileReader(void) = default;

      /// Retrieves and returns the error messages that arose during the last attempt at reading a
      /// configuration file.
      /// @return Error messages from last configuration file read attempt.
      inline const std::vector<std::wstring>& GetReadErrors(void) const
      {
        return readErrors;
      }

      /// Specifies whether or not any errors arose during the last attempt at reading a
      /// configuration files.
      /// @return `true` if so, `false` if not.
      inline bool HasReadErrors(void) const
      {
        return !(readErrors.empty());
      }

      /// Reads and parses a configuration file, storing the settings in the supplied configuration
      /// object. Intended to be invoked externally. Subclasses should not override this method.
      /// @param [in] configFileName Name of the configuration file to read.
      /// @return Configuration data object filled based on the contents of the configuration file.
      ConfigurationData ReadConfigurationFile(std::wstring_view configFileName);

    protected:

      /// Invoked at the beginning of a configuration file read operation.
      /// Subclasses are given the opportunity to initialize or reset any stored state, as
      /// needed. Overriding this method is optional, as a default implementation exists that
      /// does nothing.
      virtual void BeginRead(void);

      /// Invoked at the end of a configuration file read operation.
      /// Subclasses are given the opportunity to initialize or reset any stored state, as
      /// needed. Overriding this method is optional, as a default implementation exists that
      /// does nothing.
      virtual void EndRead(void);

      /// Sets a semantically-rich error message to be presented to the user in response to a
      /// subclass returning an error when asked what action to take. If a subclass does not set a
      /// semantically-rich error message then the default error message is used instead. Intended
      /// to be invoked optionally by subclasses during any method calls that return #EAction but
      /// only when #EAction::Error is being returned.
      /// @param [in] errorMessage String that is consumed to provide a semantically-rich error
      /// message.
      inline void SetErrorMessage(std::wstring&& errorMessage)
      {
        lastErrorMessage = std::move(errorMessage);
      }

      /// Convenience wrapper that sets a semantically-rich error message using a string view.
      /// @param [in] errorMessage String view that is copied to provide a semantically-rich error
      /// message.
      inline void SetErrorMessage(std::wstring_view errorMessage)
      {
        SetErrorMessage(std::wstring(errorMessage));
      }

      /// Convenience wrapper that sets a semantically-rich error message using a null-terminated
      /// C-style string.
      /// @param [in] errorMessage Temporary buffer containing a null-terminated string that is
      /// copied to provide a semantically-rich error message.
      inline void SetErrorMessage(const wchar_t* errorMessage)
      {
        SetErrorMessage(std::wstring(errorMessage));
      }

      /// Specifies the action to take when a given section is encountered in a configuration file
      /// (i.e. the names that typically appear in [square brackets] and separate the configuration
      /// file into namespaces).
      /// Invoked while reading from a configuration file.
      /// Subclasses must override this method. They are allowed to process the section name however
      /// they see fit and indicate to the caller what action to take.
      /// @param [in] section Name of the section, as read from the configuration file.
      /// @return Action to take with the section.
      virtual EAction ActionForSection(std::wstring_view section) = 0;

      /// Invoked to allow the subclass to process the specified integer-typed configuration
      /// setting, identified by enclosing section name and by configuration setting name.
      /// Subclasses are allowed to process the value however they see fit and indicate to the
      /// caller what action to take. Any values passed as read-only views are backed by temporary
      /// memory that will be discarded upon method return. Subclasses should copy values that need
      /// to be preserved outside of the configuration data structure.
      /// @param [in] section Name of the enclosing section, as read from the configuration file.
      /// @param [in] name Name of the configuration setting, as read from the configuration file.
      /// @param [in] value View of the value of the configuration setting, as read and parsed from
      /// the configuration file.
      /// @return Action to take with the name/value pair.
      virtual EAction ActionForValue(
          std::wstring_view section, std::wstring_view name, TIntegerView value) = 0;

      /// Invoked to allow the subclass to process the specified Boolean-typed configuration
      /// setting, identified by enclosing section name and by configuration setting name.
      /// Subclasses are allowed to process the value however they see fit and indicate to the
      /// caller what action to take. Any values passed as read-only views are backed by temporary
      /// memory that will be discarded upon method return. Subclasses should copy values that need
      /// to be preserved outside of the configuration data structure.
      /// @param [in] section Name of the enclosing section, as read from the configuration file.
      /// @param [in] name Name of the configuration setting, as read from the configuration file.
      /// @param [in] value View of the value of the configuration setting, as read and parsed from
      /// the configuration file.
      /// @return Action to take with the name/value pair.
      virtual EAction ActionForValue(
          std::wstring_view section, std::wstring_view name, TBooleanView value) = 0;

      /// Invoked to allow the subclass to process specified string-typed configuration setting,
      /// identified by enclosing section name and by configuration setting name. Subclasses are
      /// allowed to process the value however they see fit and indicate to the caller what action
      /// to take. Any values passed as read-only views are backed by temporary memory that will be
      /// discarded upon method return. Subclasses should copy values that need to be preserved
      /// outside of the configuration data structure.
      /// @param [in] section Name of the enclosing section, as read from the configuration file.
      /// @param [in] name Name of the configuration setting, as read from the configuration file.
      /// @param [in] value View of the value of the configuration setting, as read and parsed from
      /// the configuration file.
      /// @return Action to take with the name/value pair.
      virtual EAction ActionForValue(
          std::wstring_view section, std::wstring_view name, TStringView value) = 0;

      /// Specifies the type of the value for the given configuration setting.
      /// In lines that are of the form "name = value" parameters identify both the enclosing
      /// section and the name part. Subclasses should override this method. For example, if the
      /// value is expected to be an integer, subclasses should indicate this so the value is parsed
      /// and submitted correctly.
      /// @param [in] section Name of the enclosing section, as read from the configuration file.
      /// @param [in] name Name of the configuration setting (the left part of the example line
      /// given above).
      /// @return Type to associate with the value (the right part of the example line given above),
      /// which can be an error if the particular configuration setting is not supported.
      virtual EValueType TypeForValue(std::wstring_view section, std::wstring_view name) = 0;

    private:

      /// Used internally to retrieve and reset a semantically-rich error message if it exists.
      /// @return Last error message.
      inline std::wstring GetLastErrorMessage(void)
      {
        return std::move(lastErrorMessage);
      }

      /// Used internally to determine if a semantically-rich error message exists.
      /// @return `true` if an error message has been set, `false` otherwise.
      inline bool HasLastErrorMessage(void) const
      {
        return !(lastErrorMessage.empty());
      }

      /// Holds the error messages that describes any errors that occurred during
      /// configuration file read.
      std::vector<std::wstring> readErrors;

      /// Holds a semantically-rich error message to be presented to the user whenever there
      /// is an error processing a configuration value.
      std::wstring lastErrorMessage;
    };

    /// Type alias for a suggested format for storing the supported layout of a section within a
    /// configuration file. Useful for pre-determining what is allowed to appear within one section
    /// of a configuration file.
    using TConfigurationFileSectionLayout = std::map<std::wstring_view, EValueType, std::less<>>;

    /// Type alias for a suggested format for storing the supported layout of a configuration file.
    /// Useful for pre-determining what is allowed to appear within a configuration file.
    using TConfigurationFileLayout =
        std::map<std::wstring_view, TConfigurationFileSectionLayout, std::less<>>;
  } // namespace Configuration
} // namespace Xidi
