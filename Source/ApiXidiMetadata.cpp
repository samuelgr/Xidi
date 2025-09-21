/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ApiXidiMetadata.cpp
 *   Implementation of the Metadata interface part of the Xidi API.
 **************************************************************************************************/

#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiXidi.h"
#include "Strings.h"

namespace Xidi
{
  namespace Api
  {
    /// Implements the Xidi API interface #IMetadata.
    class MetadataProvider : public IMetadata
    {
    public:

      // IMetadata
      Infra::ProcessInfo::SVersionInfo GetVersion(void) const override
      {
        return Infra::ProcessInfo::GetProductVersion();
      }

      std::wstring_view GetFormName(void) const override
      {
        return Strings::GetFormName();
      }
    };

    // Singleton Xidi API implementation object.
    static MetadataProvider metadataProvider;
  } // namespace Api
} // namespace Xidi
