@echo off
setlocal enabledelayedexpansion

rem +--------------------------------------------------------------------------
rem | Xidi
rem |   DirectInput interface for XInput controllers.
rem +--------------------------------------------------------------------------
rem | Authored by Samuel Grossman
rem | Copyright (c) 2016-2022
rem +--------------------------------------------------------------------------
rem | PackageRelease.bat
rem |   Script for packaging up a release. To be executed manually after
rem |   building the Release configuration for both Win32 and x64 platforms.
rem +--------------------------------------------------------------------------

set project_name=Xidi
set project_platforms=Win32 x64

set project_has_sdk=no
set project_has_third_party_license=yes

set files_release=LICENSE README.md
set files_release_build=dinput.dll dinput8.dll winmm.dll
set files_release_build_Win32=Xidi.HookModule.32.dll
set files_release_build_x64=Xidi.HookModule.64.dll

set third_party_license=Hookshot Boost XstdBitSet

rem ---------------------------------------------------------------------------

set script_path=%~dp0
set script_invocation=%0
shift

set version_info_extra_args=
set build_output_subdirectory=Release
set assume_always_yes=no

echo.
echo Release Packager for %project_name%
echo.

:next_arg

if "%0"=="" (
    goto :done_args
) else if "%0"=="/?" (
    goto :usage
) else if "%0"=="/d" (
    set version_info_extra_args=debug
    set build_output_subdirectory=Debug
) else if "%0"=="/o" (
    set output_dir=%~f1
    if "!output_dir!"=="" (
        echo Missing output directory.
        goto :try_help_message_and_exit
    )
    shift
) else if "%0"=="/y" (
    set assume_always_yes=yes
) else (
    echo Unrecognized argument: %0
    goto :try_help_message_and_exit
)

shift
goto :next_arg

:done_args

set version_info_script=%script_path%\GitVersionInfo.bat
set version_info_dir=%script_path%Output
set version_info_file=%version_info_dir%\GitVersionInfo.h

call "%version_info_script%" "%version_info_dir%" %version_info_extra_args% >NUL 2>NUL

if not exist %version_info_file% (
    echo Failed to generate information file: %version_info_file%
    goto :exit_with_error
)

for /f "usebackq tokens=3" %%V in (`findstr GIT_VERSION_STRING %version_info_file%`) do set raw_release_ver=%%~V
if "%raw_release_ver%"=="" (
    echo Missing release version^^!
    goto :exit_with_error
)

for /f "usebackq" %%V in (`echo %raw_release_ver% ^| findstr ^^[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*`) do set release_ver=%%~V
for /f "usebackq" %%V in (`echo %release_ver% ^| findstr unknown`) do set release_ver=
if "%release_ver%"=="" (
    echo Invalid release version^^!
    goto :exit_with_error
)

if "%output_dir%"=="" (
    for /f "usebackq tokens=3*" %%D in (`reg query "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders" /v Desktop`) do if "%%~E"=="" (set output_dir=%%~D) else (set output_dir=%%~D %%~E)
    call set output_dir=!output_dir!
)
if "%output_dir%"=="" (
    echo Missing output directory^^!
    goto :exit_with_error
)

set output_dir=%output_dir%\%project_name%-v%release_ver%

echo Version:
echo     %raw_release_ver%

echo.
echo Output directory:
echo     %output_dir%

if not yes==%assume_always_yes% (
    echo.
    choice /M "Proceed?"
    if not !ERRORLEVEL!==1 exit /b 2
)

if exist %output_dir% (
    echo.
    echo Output directory exists and will be overwritten.
    if not yes==%assume_always_yes% (
        choice /M "Still proceed?"
        if not !ERRORLEVEL!==1 exit /b 2
    )
    rd /S /Q %output_dir%
)

echo.
echo Copied files:

pushd %script_dir%
md %output_dir%
for %%F in (%files_release%) do (
    if exist %%F (
        echo     %%F
        copy "%%F" "%output_dir%" >NUL 2>NUL
    ) else (
        set files_are_missing=yes
    )
)
for %%P in (%project_platforms%) do (
	if not ""=="%files_release_build%!files_release_build_%%P!" (
        md %output_dir%\%%P

        for %%F in (%files_release_build% !files_release_build_%%P!) do (
            if exist Output\%%P\%build_output_subdirectory%\%%F (
                echo     Output\%%P\%build_output_subdirectory%\%%F
                copy "Output\%%P\%build_output_subdirectory%\%%F" "%output_dir%\%%P" >NUL 2>NUL
            ) else (
                set files_are_missing=yes
            )
        )
    )
)
if "yes"=="%project_has_sdk%" (
    md %output_dir%\SDK

    if not ""=="%files_sdk_lib%" (
        md %output_dir%\SDK\Lib
        for %%F in (%files_sdk_lib%) do (
            if exist %%F (
                echo     %%F
                copy "%%F" "%output_dir%\SDK\Lib" >NUL 2>NUL
            ) else (
                set files_are_missing=yes
            )
        )
    )

    if not ""=="%files_sdk_include%" (
        md %output_dir%\SDK\Include
        md %output_dir%\SDK\Include\%project_name%
        for %%F in (%files_sdk_include%) do (
            if exist %%F (
                echo     %%F
                copy "%%F" "%output_dir%\SDK\Include\%project_name%" >NUL 2>NUL
            ) else (
                set files_are_missing=yes
            )
        )
    )
)
if "yes"=="%project_has_third_party_license%" (
    md %output_dir%\ThirdParty
    for %%T in (%third_party_license%) do (
        if exist ThirdParty\%%T\LICENSE (
            echo     ThirdParty\%%T\LICENSE
            copy "ThirdParty\%%T\LICENSE" "%output_dir%\ThirdParty\%%T_LICENSE" >NUL 2>NUL
        ) else (
            set files_are_missing=yes
        )
    )
)
popd

if "yes"=="%files_are_missing%" (
    echo.
    echo Missing files:
    
    pushd %script_dir%
    for %%F in (%files_release% %files_sdk_lib% %files_sdk_include%) do (
        if not exist %%F (
            echo     %%F
        )
    )
    for %%P in (%project_platforms%) do (
        if not ""=="%files_release_build%!files_release_build_%%P!" (
            for %%F in (%files_release_build% !files_release_build_%%P!) do (
                if not exist Output\%%P\%build_output_subdirectory%\%%F (
                    echo     Output\%%P\%build_output_subdirectory%\%%F
                )
            )
        )
    )
    if "yes"=="%project_has_third_party_license%" (
        for %%T in (%third_party_license%) do (
            if not exist ThirdParty\%%T\LICENSE (
                echo     ThirdParty\%%T\LICENSE
            )
        )
    )
    popd
)

:exit
echo.
if not yes==%assume_always_yes% (
    pause
)
exit /b 0

:exit_with_error
exit /b 1

:try_help_message_and_exit
echo Try "/?" for help.
goto :exit_with_error

:usage
echo Usage: %script_invocation% [/d] [/o ^<output-dir^>] [/y]
echo   /d
echo     Use the debug build instead of the release build.
echo   /o ^<output-dir^>
echo     Place the output release package in the specified directory.
echo     Other directories in the hierarchy will be created as needed,
echo     and final output will go in a subdirectory of the specified
echo     output directory. Default output directory is the current user's
echo     desktop.
echo   /y
echo     Run non-interactively and assume "Y" is the response to all input
echo     prompts, including overwriting the output directory if it exists.
exit /b 0
