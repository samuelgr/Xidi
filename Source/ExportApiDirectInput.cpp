/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ExportApiDirectInput.cpp
 *   Implementation of primary exported functions for the DirectInput
 *   library.
 *****************************************************************************/

#include "DirectInputClassFactory.h"
#include "ImportApiDirectInput.h"
#include "Message.h"
#include "WrapperIDirectInput.h"

using namespace Xidi;


// -------- HELPERS -------------------------------------------------------- //

/// Logs an error event indicating that an instance of IDirectInput(8) could not be created due to a version out-of-range error.
/// @param [in] minVersion Minimum allowed version.
/// @param [in] maxVersion Maximum allowed version.
/// @param [in] receivedVersion Actual version received.
inline static void LogVersionOutOfRange(DWORD minVersion, DWORD maxVersion, DWORD receivedVersion)
{
    Message::OutputFormatted(Message::ESeverity::Error, L"Failed to create a DirectInput interface object because the version is out of range (expected 0x%04x to 0x%04x, got 0x%04x).", minVersion, maxVersion, receivedVersion);
}

/// Logs an error event indicating that an instance of IDirectInput(8) could not be created due to an invalid interface parameter.
inline static void LogInvalidInterfaceParam(void)
{
    Message::Output(Message::ESeverity::Error, L"Failed to create a DirectInput interface object because the requested IID is invalid.");
}

/// Logs an error event indicating that an instance of IDirectInput(8) could not be created due to an error having been returned by the system.
/// @param [in] errorCode Error code returned by the system.
inline static void LogSystemCreateError(HRESULT errorCode)
{
    Message::OutputFormatted(Message::ESeverity::Error, L"Failed to create a DirectInput interface object because the imported function returned code 0x%08x.", errorCode);
}

/// Logs an informational event indicating that an instance of IDirectInput(8) was created successfully.
inline static void LogSystemCreateSuccess(void)
{
    Message::Output(Message::ESeverity::Info, L"Successfully created a DirectInput interface object.");
}


extern "C"
{
    // -------- DLL EXPORT FUNCTIONS --------------------------------------- //
    // See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
    HRESULT WINAPI ExportApiDirectInputDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
        void* diObject = nullptr;

        if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        {
            LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
            return DIERR_INVALIDPARAM;
        }

        if ((IID_IDirectInput8W != riidltf) && (IID_IDirectInput8A != riidltf))
        {
            LogInvalidInterfaceParam();
            return DIERR_INVALIDPARAM;
        }

        HRESULT result = ImportApiDirectInput::DirectInput8Create(hinst, dwVersion, riidltf, &diObject, punkOuter);
        if (DI_OK != result)
        {
            LogSystemCreateError(result);
            return result;
        }

        if (IID_IDirectInput8W == riidltf)
            diObject = new WrapperIDirectInput<true>((IDirectInput8W*)diObject);
        else
            diObject = new WrapperIDirectInput<false>((IDirectInput8A*)diObject);

        *ppvOut = (LPVOID)diObject;

        LogSystemCreateSuccess();
        return result;
    }
#else
    HRESULT WINAPI ExportApiDirectInputDirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
    {
        IDirectInputA* diObject = nullptr;

        if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        {
            LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
            return E_FAIL;
        }

        HRESULT result = ImportApiDirectInput::DirectInputCreateA(hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTA*)&diObject, punkOuter);
        if (DI_OK != result)
        {
            LogSystemCreateError(result);
            return result;
        }

        diObject = new WrapperIDirectInput<false>((LatestIDirectInputA*)diObject);
        *ppDI = (LPDIRECTINPUTA)diObject;

        LogSystemCreateSuccess();
        return result;
    }

    // ---------

    HRESULT WINAPI ExportApiDirectInputDirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
    {
        IDirectInput* diObject = nullptr;

        if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        {
            LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
            return E_FAIL;
        }

        HRESULT result = ImportApiDirectInput::DirectInputCreateW(hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTW*)&diObject, punkOuter);
        if (DI_OK != result)
        {
            LogSystemCreateError(result);
            return result;
        }

        diObject = new WrapperIDirectInput<true>((LatestIDirectInputW*)diObject);
        *ppDI = (LPDIRECTINPUTW)diObject;

        LogSystemCreateSuccess();
        return result;
    }

    // ---------

    HRESULT WINAPI ExportApiDirectInputDirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
    {
        void* diObject = nullptr;

        if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        {
            LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
            return E_FAIL;
        }

        if ((IID_IDirectInputW != riidltf) && (IID_IDirectInput2W != riidltf) && (IID_IDirectInput7W != riidltf) && (IID_IDirectInputA != riidltf) && (IID_IDirectInput2A != riidltf) && (IID_IDirectInput7A != riidltf))
        {
            LogInvalidInterfaceParam();
            return DIERR_INVALIDPARAM;
        }

        HRESULT result;
        BOOL useUnicode = FALSE;

        if (IID_IDirectInputW == riidltf || IID_IDirectInput2W == riidltf || IID_IDirectInput7W == riidltf)
        {
            useUnicode = TRUE;
            result = ImportApiDirectInput::DirectInputCreateEx(hinst, DIRECTINPUT_VERSION, IID_IDirectInput7W, (LPVOID*)&diObject, punkOuter);
        }
        else
        {
            result = ImportApiDirectInput::DirectInputCreateEx(hinst, DIRECTINPUT_VERSION, IID_IDirectInput7A, (LPVOID*)&diObject, punkOuter);
        }

        if (DI_OK != result)
        {
            LogSystemCreateError(result);
            return result;
        }

        if (TRUE == useUnicode)
            diObject = new WrapperIDirectInput<true>((LatestIDirectInputW*)diObject);
        else
            diObject = new WrapperIDirectInput<false>((LatestIDirectInputA*)diObject);

        *ppvOut = diObject;

        LogSystemCreateSuccess();
        return result;
    }
    #endif

    // ---------

    HRESULT WINAPI ExportApiDirectInputDllRegisterServer(void)
    {
        return ImportApiDirectInput::DllRegisterServer();
    }

    // ---------

    HRESULT WINAPI ExportApiDirectInputDllUnregisterServer(void)
    {
        return ImportApiDirectInput::DllUnregisterServer();
    }

    // ---------

    HRESULT WINAPI ExportApiDirectInputDllCanUnloadNow(void)
    {
        // In practice, there should not be a need to unload Xidi or DirectInput DLLs.
        return S_FALSE;
    }

    // ---------

    HRESULT WINAPI ExportApiDirectInputDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
    {
        if (DirectInputClassFactory::CanCreateObjectsOfClass(rclsid))
        {
            if (IsEqualIID(IID_IClassFactory, riid))
            {
                Message::Output(Message::ESeverity::Info, L"DllGetClassObject is intercepting the class object request because the class ID is supported and the interface ID is correct.");
                *ppv = DirectInputClassFactory::GetInstance();
                return S_OK;
            }
            else
            {
                Message::Output(Message::ESeverity::Warning, L"DllGetClassObject failed to intercept the class object request because the class ID is supported but the interface ID is incorrect.");
                return E_NOINTERFACE;
            }
        }

        Message::Output(Message::ESeverity::Info, L"DllGetClassObject is not intercepting the class object request because the class ID is unsupported.");
        return ImportApiDirectInput::DllGetClassObject(rclsid, riid, ppv);
    }
}
