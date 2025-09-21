/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ApiXidiImportFunctions2.cpp
 *   Implementation of the ImportFunctions2 part of the Xidi API.
 **************************************************************************************************/

#include <unordered_map>

#include "ApiXidi.h"
#include "ImportApiDirectInput.h"
#include "ImportApiWinMM.h"

namespace Xidi
{
  namespace Api
  {
    /// Implements the Xidi API interface #IImportFunctions2.
    class ImportFunctionsReplacer : public IImportFunctions2
    {
    public:

      // IImportFunctions2
      const std::unordered_map<std::wstring_view, size_t>* GetReplaceable(
          ELibrary library) const override
      {
        const auto mutableImportTableIter = mutableImportTablesByLibrary.find(library);
        if (mutableImportTablesByLibrary.cend() == mutableImportTableIter) return nullptr;
        return &mutableImportTableIter->second->GetReplaceable();
      }

      size_t SetReplaceable(
          ELibrary library,
          const std::unordered_map<std::wstring_view, const void*>& importFunctionTable) override
      {
        const auto mutableImportTableIter = mutableImportTablesByLibrary.find(library);
        if (mutableImportTablesByLibrary.cend() == mutableImportTableIter) return 0;

        IMutableImportTable* mutableImportTable = mutableImportTableIter->second;
        size_t numReplacedFunctions = 0;
        for (const auto& replacementFunction : importFunctionTable)
        {
          if (true ==
              mutableImportTable->SetReplaceable(
                  replacementFunction.first, replacementFunction.second))
            numReplacedFunctions += 1;
        }

        return numReplacedFunctions;
      }

    private:

      /// Map from library to associated mutable import table interface.
      static const std::unordered_map<ELibrary, IMutableImportTable*> mutableImportTablesByLibrary;
    };

    const std::unordered_map<IImportFunctions2::ELibrary, IMutableImportTable*>
        ImportFunctionsReplacer::mutableImportTablesByLibrary = {
            {IImportFunctions2::ELibrary::DInput,
             ImportApiDirectInput::VersionLegacy::GetMutableImportTable()},
            {IImportFunctions2::ELibrary::DInput8,
             ImportApiDirectInput::Version8::GetMutableImportTable()},
            {IImportFunctions2::ELibrary::WinMM, ImportApiWinMM::GetMutableImportTable()}};

    // Singleton Xidi API implementation object.
    static ImportFunctionsReplacer importFunctionsReplacer;
  } // namespace Api
} // namespace Xidi
