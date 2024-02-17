@echo off
setlocal enabledelayedexpansion

rem +--------------------------------------------------------------------------
rem | Xidi
rem |   DirectInput interface for XInput controllers.
rem +--------------------------------------------------------------------------
rem | Authored by Samuel Grossman
rem | Copyright (c) 2016-2024
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
set digitally_sign_binaries=no
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
) else if "%0"=="/s" (
    set digitally_sign_binaries=yes
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

if not yes==%digitally_sign_binaries% (
    set is_official_build=no

    for /f "usebackq tokens=3" %%V in (`findstr GIT_VERSION_COMMIT_DISTANCE %version_info_file%`) do set git_commit_distance=%%~V
    if "!git_commit_distance!"=="0" (
        for /f "usebackq tokens=3" %%V in (`findstr GIT_VERSION_IS_DIRTY %version_info_file%`) do set git_is_dirty=%%~V
        if "!git_is_dirty!"=="0" (
            set is_official_build=yes
        )
    )
    if !is_official_build!==yes (
        if not yes==%assume_always_yes% (
            echo.
            choice /M "Sign binaries?"
            if !ERRORLEVEL!==1 set digitally_sign_binaries=yes
        )
    )
)

if exist %output_dir% (
    echo.
    echo Output directory exists and will be overwritten.
    if not yes==%assume_always_yes% (
        choice /M "Proceed anyway?"
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
            if exist Output\%build_output_subdirectory%\%%P\%%F (
                echo     Output\%build_output_subdirectory%\%%P\%%F
                copy "Output\%build_output_subdirectory%\%%P\%%F" "%output_dir%\%%P" >NUL 2>NUL
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
                if not exist Output\%build_output_subdirectory%\%%P\%%F (
                    echo     Output\%build_output_subdirectory%\%%P\%%F
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
    if "yes"=="%digitally_sign_binaries%" (
        echo.
        echo Digital signatures were requested but output files are missing.
        if not yes==%assume_always_yes% (
            choice /M "Sign anyway?"
            if not !ERRORLEVEL!==1 set digitally_sign_binaries=no
        )
    )
)

if yes==%digitally_sign_binaries% (
    pushd %output_dir%

    set signtool_flags=/fdchw /tdchw /f "%CODE_SIGN_CERTIFICATE_PFX_FILE%" /fd "%CODE_SIGN_DIGEST_ALGORITHM%"
    set uses_timestamp_countersignature=no
    if not "%CODE_SIGN_CERTIFICATE_PFX_PASSWORD%"=="" (
        set signtool_flags=!signtool_flags! /p %CODE_SIGN_CERTIFICATE_PFX_PASSWORD%
    )
    if not "%CODE_SIGN_TIMESTAMP_SERVER%"=="" (
        set signtool_flags=!signtool_flags! /tr "%CODE_SIGN_TIMESTAMP_SERVER%" /td "%CODE_SIGN_DIGEST_ALGORITHM%"
        set uses_timestamp_countersignature=yes
    )

    set vcvars_batch_file=
    set found_signtool=no
    if exist "%ProgramFiles%\Microsoft Visual Studio\2022" (
        pushd "%ProgramFiles%\Microsoft Visual Studio\2022"
        for /f "usebackq tokens=*" %%B in (`dir /S /B vcvars64.bat`) do set vcvars_batch_file=%%B
        popd
    )
    if not "!vcvars_batch_file!"=="" (
        call "!vcvars_batch_file!" >NUL 2>NUL

        where signtool > NUL 2>NUL
        if !ERRORLEVEL!==0 (
            set found_signtool=yes
        )
    )

    if "!found_signtool!"=="yes" (
        echo.
        echo Digitally signing files:

        set sign_suffixes=exe dll

        set sign_search_paths=
        for %%E in (!sign_suffixes!) do (
            set sign_search_paths="*.%%E" !sign_search_paths!
        )
        for %%P in (%project_platforms%) do (
            for %%E in (!sign_suffixes!) do (
                set sign_search_paths="%%P\*.%%E" !sign_search_paths!
            )
        )

        set signed_a_file=no
        for %%B in (!sign_search_paths!) do (
            set signed_a_file=yes

            signtool sign !signtool_flags! %%B >%%B.signtool.log.txt 2>&1
            if !ERRORLEVEL!==0 (
                del %%B.signtool.log.txt >NUL 2>NUL
                echo     %%B

                if "yes"=="!uses_timestamp_countersignature!" (
                    timeout /t 15 /nobreak >NUL 2>NUL
                )
            ) else (
                echo     %%B ^(failed^)
            )
        )

        if "no"=="!signed_a_file!" (
            echo     ^(none^)
        )
    ) else (
        echo.
        echo Unable to digitally sign files because "signtool" could not be located.
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
echo Usage: %script_invocation% ^<args^>
echo   /d
echo     Use the debug build instead of the release build.
echo   /o ^<output-dir^>
echo     Place the output release package in the specified directory.
echo     Other directories in the hierarchy will be created as needed,
echo     and final output will go in a subdirectory of the specified
echo     output directory. Default output directory is the current user's
echo     desktop.
echo   /s
echo     Attempt to digitally sign binary files that are packaged for
echo     release. This applies to files with extensions DLL and EXE.
echo   /y
echo     Run non-interactively and assume "Y" is the response to all input
echo     prompts, including overwriting the output directory if it exists.
exit /b 0
