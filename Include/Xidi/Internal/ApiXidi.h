/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ApiXidi.h
 *   Declaration of an internal API for communication between Xidi modules.
 **************************************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <Infra/Core/ProcessInfo.h>

namespace Xidi
{
  namespace Api
  {
    /// Enumerates all available API classes.
    /// Once created and released an API class cannot be modified. However, it can be extended
    /// through inheritance. Order of enumerators also cannot be changed.
    enum class EClass : unsigned int
    {
      /// IMetadata
      Metadata,

      /// IImportFunctions - deprecated
      ImportFunctions,

      /// IImportFunctions2
      ImportFunctions2
    };

    /// Xidi API base class. All API classes must inherit from this class.
    class IXidi
    {
    public:

      /// Typically, concrete API implementations are singleton objects. When constructed they are
      /// expected to be associated with a particular API class enumerator as the API implementation
      /// provider object.
      IXidi(EClass apiClass);

      virtual ~IXidi(void) = default;
    };

    /// Xidi API class for obtaining metadata about the running Xidi module.
    /// Guaranteed to be implemented and available in all Xidi modules.
    class IMetadata : public IXidi
    {
    public:

      /// Retrieves and returns the version information structure of the running Xidi module.
      /// @return Filled-in version information structure.
      virtual Infra::ProcessInfo::SVersionInfo GetVersion(void) const = 0;

      /// Retrieves and returns a string that identifies the running form of Xidi.
      /// @return String identifying the form of Xidi.
      virtual std::wstring_view GetFormName(void) const = 0;

    protected:

      inline IMetadata(void) : IXidi(EClass::Metadata) {}
    };

    /// Xidi API class for manipulating the functions Xidi imports from the system.
    class IImportFunctions2 : public IXidi
    {
    public:

      /// Enumeration of libraries for which import function replacement is supported.
      enum class ELibrary
      {
        DInput,
        DInput8,
        WinMM,
        XInput
      };

      /// Retrieves a list of names of imported functions whose import addresses can be replaced.
      /// Xidi imports some of its functionality from the system, but in some cases these import
      /// locations need to be changed. Function names returned in the read-only view are also
      /// exported by Xidi, and their addresses can be retrieved using `GetProcAddress` directly.
      /// @param [in] library Library for which replaceable functions are requested.
      /// @return Read-only pointer to a map from function names to internal import table indices,
      /// or `nullptr` if the specified library does not have replaceable imported functions.
      virtual const std::unordered_map<std::wstring_view, size_t>* GetReplaceable(
          ELibrary library) const = 0;

      /// Submits to Xidi a set of replacement import function addresses as a map from name to
      /// address. Xidi imports some of its functionality from the system, but in some cases these
      /// import locations need to be changed. Valid function names are obtained using the
      /// #GetReplaceable API function.
      /// @param [in] library Library for which functions are to be replaced.
      /// @param [in] importFunctionTable Read-only reference to a map from function name to
      /// replacement import address.
      /// @return Number of functions whose addresses were successfully replaced using the provided
      /// import function table.
      virtual size_t SetReplaceable(
          ELibrary library,
          const std::unordered_map<std::wstring_view, const void*>& importFunctionTable) = 0;

    protected:

      inline IImportFunctions2(void) : IXidi(EClass::ImportFunctions2) {}
    };

    /// Interface for accessing and replacing the functions for a single library's import table.
    class IMutableImportTable
    {
    public:

      /// Retrieves all replaceable imported functions, mapped from name to index in the internal
      /// import table.
      /// @return Read-only reference to replaceable imported functions.
      virtual const std::unordered_map<std::wstring_view, size_t>& GetReplaceable(void) const = 0;

      /// Replaces the specified imported function with a new one, identified by name.
      /// @param [in] replaceableFunctionName Name of the function whose import address is to be
      /// replaced.
      /// @param [in] newAddress New address to use for the imported function.
      /// @return `true` if the replacement was successful, `false` otherwise.
      inline bool SetReplaceable(std::wstring_view replaceableFunctionName, const void* newAddress)
      {
        const auto replaceableFunction = GetReplaceable().find(replaceableFunctionName);
        if (GetReplaceable().cend() == replaceableFunction) return false;
        return SetReplaceable(replaceableFunction->second, newAddress);
      }

      /// Replaces the specified imported function with a new one, identified by index into the
      /// import table.
      /// @param [in] replaceableFunctionIndex Index in the internal import table of the function
      /// whose import address is to be replaced.
      /// @param [in] newAddress New address to use for the imported function.
      /// @return `true` if the replacement was successful, `false` otherwise.
      virtual bool SetReplaceable(size_t replaceableFunctionIndex, const void* newAddress) = 0;
    };

    /// Pointer type definition for the XidiApiGetInterface exported function.
    using TGetInterfaceFunc = IXidi* (*)(EClass apiClass);

    /// Constant for the name of the XidiApiGetInterface exported function.
    inline constexpr const char* const kGetInterfaceFuncName = "XidiApiGetInterface";
  } // namespace Api
} // namespace Xidi
