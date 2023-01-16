/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file DirectInputClassFactory.h
 *   Declaration of COM class factory functionality for DirectInput objects.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


namespace Xidi
{
    /// Factory class for constructing DirectInput COM objects which implements the standard COM class factory interface for doing so.
    /// Internally this is just a singleton object, so all reference count operations are no-ops.
    class DirectInputClassFactory : public IClassFactory
    {
    public:
        // -------- CLASS METHODS ------------------------------------------ //

        /// Checks if this factory class can create objects of the specified COM class ID.
        /// @param [in] rclsid Class ID of the COM class to check.
        /// @return `true` if objects of the specified class ID can be created, `false` otherwise.
        static bool CanCreateObjectsOfClass(REFCLSID rclsid);

        /// Retrieves the singleton instance of this class as a standard COM interface pointer.
        /// @return COM interface pointer.
        static IClassFactory* GetInstance(void);


    private:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor.
        DirectInputClassFactory(void) = default;

        /// Copy constructor. Should never be invoked.
        DirectInputClassFactory(const DirectInputClassFactory& other) = delete;


    public:
        // -------- METHODS: IUnknown -------------------------------------- //
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
        ULONG STDMETHODCALLTYPE AddRef(void) override;
        ULONG STDMETHODCALLTYPE Release(void) override;

        // -------- METHODS: IClassFactory --------------------------------- //
        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override;
        HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) override;
    };
}
