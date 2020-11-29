/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file CoCreateInstance.cpp
 *   Implementation of hook for CoCreateInstance.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiWindows.h"
#include "Hooks.h"
#include "Message.h"
#include "Strings.h"

#include <string>
#include <string_view>


namespace Xidi
{
    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Creates an instance of the specified COM object by invoking the provided DllGetClassObject procedure to retrieve the class factory object.
    /// See IClassFactory::CreateInstance documentation for more information on parameters and return values.
    static HRESULT CreateInstance(decltype(&DllGetClassObject) procDllGetClassObject, REFCLSID rclsid, LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppv)
    {
        if (nullptr == procDllGetClassObject)
        {
            Message::Output(Message::ESeverity::Error, L"CreateInstance encountered an internal null pointer error while attempting to create an instance of a COM object.");
            return E_FAIL;
        }

        if (Message::WillOutputMessageOfSeverity(Message::ESeverity::Debug))
        {
            LPOLESTR clsidString = nullptr;
            LPOLESTR iidString = nullptr;

            const HRESULT clsidStringResult = StringFromCLSID(rclsid, &clsidString);
            const HRESULT iidStringResult = StringFromIID(riid, &iidString);

            Message::OutputFormatted(Message::ESeverity::Debug, L"CreateInstance is attempting to create a COM object instance with CLSID=%s and IID=%s", (S_OK == clsidStringResult ? clsidString : L"(unknown)"), (S_OK == iidStringResult ? iidString : L"(unknown)"));

            if (S_OK == clsidStringResult)
                CoTaskMemFree(clsidString);

            if (S_OK == iidStringResult)
                CoTaskMemFree(iidString);
        }

        IClassFactory* comClassObject = nullptr;
        const HRESULT comClassObjectResult = procDllGetClassObject(rclsid, IID_IClassFactory, (LPVOID*)&comClassObject);

        if (S_OK != comClassObjectResult)
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"CreateInstance failed with HRESULT code 0x%08x to create a class factory object via DllGetClassObject.", (unsigned int)comClassObjectResult);
            return comClassObjectResult;
        }

        LPVOID newObject = nullptr;
        const HRESULT newObjectResult = comClassObject->CreateInstance(pUnkOuter, riid, &newObject);

        comClassObject->Release();

        if (S_OK != newObjectResult)
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"CreateInstance failed with HRESULT code 0x%08x to create an instance of the requested object.", (unsigned int)newObjectResult);
            return newObjectResult;
        }

        Message::Output(Message::ESeverity::Debug, L"CreateInstance successfully created an instance of a COM object.");
        *ppv = newObject;
        return S_OK;
    }

    /// Retrieves the address of the DllGetClassObject function exported by the specified module, which should be located in the same directory as this hook module.
    /// @tparam moduleName Compile-time constant string of the base name of the module.
    /// @return Address of the desired procedure, or `nullptr` on failure.
    template <const std::wstring_view* moduleName> static decltype(&DllGetClassObject) LocateDllGetClassObjectProc(void)
    {
        static const std::wstring importLibraryFilename(std::wstring(Strings::kStrExecutableDirectoryName) + std::wstring(*moduleName));
        static const HMODULE moduleHandle = LoadLibrary(importLibraryFilename.c_str());
        static const FARPROC moduleDllGetClassObjectProc = GetProcAddress(moduleHandle, "DllGetClassObject");

        Message::OutputFormatted(Message::ESeverity::Debug, L"LocateDllGetClassObjectProc is attempting to locate procedure DllGetClassObject in library %s.", importLibraryFilename.c_str());

        if (nullptr == moduleHandle)
            Message::OutputFormatted(Message::ESeverity::Warning, L"LocateDllGetClassObjectProc is unable to load library %s.", importLibraryFilename.c_str());
        else if (nullptr == moduleDllGetClassObjectProc)
            Message::OutputFormatted(Message::ESeverity::Warning, L"LocateDllGetClassObjectProc is unable to locate DllGetClassObject procedure in library %s.", importLibraryFilename.c_str());

        return (decltype(&DllGetClassObject))moduleDllGetClassObjectProc;
    }
}


// -------- HOOK FUNCTION -------------------------------------------------- //
// See original function and Hookshot documentation for details.

HRESULT StaticHook_CoCreateInstance::Hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
    if (IsEqualCLSID(CLSID_DirectInput8, rclsid))
    {
        static const decltype(&DllGetClassObject) procDllGetClassObject = Xidi::LocateDllGetClassObjectProc<&Xidi::Strings::kStrLibraryNameDirectInput8>();
        
        if (nullptr != procDllGetClassObject)
        {
            Xidi::Message::Output(Xidi::Message::ESeverity::Info, L"CoCreateInstance is intercepting creation of object of type CLSID_DirectInput8.");
            return Xidi::CreateInstance(procDllGetClassObject, rclsid, pUnkOuter, riid, ppv);
        }
        else
        {
            Xidi::Message::Output(Xidi::Message::ESeverity::Warning, L"CoCreateInstance failed to intercept creation of object of type CLSID_DirectInput8 because a required procedure could not be located.");
        }
    }
    else if (IsEqualCLSID(CLSID_DirectInput, rclsid))
    {
        static const decltype(&DllGetClassObject) procDllGetClassObject = Xidi::LocateDllGetClassObjectProc<&Xidi::Strings::kStrLibraryNameDirectInput>();

        if (nullptr != procDllGetClassObject)
        {
            Xidi::Message::Output(Xidi::Message::ESeverity::Info, L"CoCreateInstance is intercepting creation of object of type CLSID_DirectInput.");
            return Xidi::CreateInstance(procDllGetClassObject, rclsid, pUnkOuter, riid, ppv);
        }
        else
        {
            Xidi::Message::Output(Xidi::Message::ESeverity::Warning, L"CoCreateInstance failed to intercept creation of object of type CLSID_DirectInput because a required procedure could not be located.");
        }
    }
    else if (IsEqualCLSID(CLSID_DirectInputDevice8, rclsid) || IsEqualCLSID(CLSID_DirectInputDevice, rclsid))
    {
        Xidi::Message::Output(Xidi::Message::ESeverity::Warning, L"CoCreateInstance is not intercepting creation of object of type CLSID_DirectInputDevice8 or CLSID_DirectInputDevice because support for these classes is not implemented.");
    }
    
    return Original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}
