/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file DirectInputClassFactory.h
 *   Declaration of COM class factory functionality for DirectInput objects.
 **************************************************************************************************/

#pragma once

#include "ApiWindows.h"

namespace Xidi
{
  /// Factory class for constructing DirectInput COM objects which implements the standard COM class
  /// factory interface for doing so. Internally this is just a singleton object, so all reference
  /// count operations are no-ops.
  class DirectInputClassFactory : public IClassFactory
  {
  public:

    /// Checks if this factory class can create objects of the specified COM class ID.
    /// @param [in] rclsid Class ID of the COM class to check.
    /// @return `true` if objects of the specified class ID can be created, `false` otherwise.
    static bool CanCreateObjectsOfClass(REFCLSID rclsid);

    /// Retrieves the singleton instance of this class as a standard COM interface pointer.
    /// @return COM interface pointer.
    static IClassFactory* GetInstance(void);

    // IClassFactory
    HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall LockServer(BOOL fLock) override;

    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObj) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

  private:

    DirectInputClassFactory(void) = default;

    DirectInputClassFactory(const DirectInputClassFactory& other) = delete;
  };
} // namespace Xidi
