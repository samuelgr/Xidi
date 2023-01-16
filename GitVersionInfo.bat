@echo off
setlocal enabledelayedexpansion

rem +--------------------------------------------------------------------------
rem | Xidi
rem |   DirectInput interface for XInput controllers.
rem +--------------------------------------------------------------------------
rem | Authored by Samuel Grossman
rem | Copyright (c) 2016-2023
rem +--------------------------------------------------------------------------
rem | GitVersionInfo.bat
rem |   Script for extracting version information from Git. Executed
rem |   automatically on build.
rem +--------------------------------------------------------------------------

set script_path=%~dp0
set output_dir=%~f1

if not "%2"=="" set version_build_metadata=%2.

if "%output_dir%"=="" (
    set output_dir=%script_path%\Output
)

set output_file=%output_dir%\GitVersionInfo.h

if not exist %output_dir% md %output_dir%
if exist %output_file% del /f %output_file%

git --version >NUL 2>NUL
if %ERRORLEVEL%==0 (
    set version_is_dirty=0
    for /f "usebackq tokens=1" %%D in (`git diff --shortstat`) do if not "%%~D"=="" set version_is_dirty=1

    for /f "usebackq delims=v tokens=1" %%V in (`git tag --list v[0-9]*.[0-9]*.[0-9]* --merged HEAD`) do set merged_release_ver=%%~V
    if not "!merged_release_ver!"=="" (
        for /f "usebackq delims=.- tokens=1" %%V in (`echo !merged_release_ver!`) do set version_major=%%~V
        for /f "usebackq delims=.- tokens=2" %%V in (`echo !merged_release_ver!`) do set version_minor=%%~V
        for /f "usebackq delims=.- tokens=3" %%V in (`echo !merged_release_ver!`) do set version_patch=%%~V
        for /f "usebackq" %%V in (`git rev-list --count v!merged_release_ver!..HEAD`) do set version_commit_distance=%%~V
    ) else (
        echo No prior version tag could be located. Unable to determine version information.

        set version_major=0
        set version_minor=0
        set version_patch=0
        set version_commit_distance=0
        set version_build_metadata=unknown.!version_build_metadata!
    )

    if "!version_commit_distance!!version_is_dirty!"=="00" (
        set version_string=!version_major!.!version_minor!.!version_patch!
    ) else (
        for /f "usebackq tokens=1" %%V in (`git describe --abbrev^=8 --always`) do (
            set /a version_patch_plus_one=!version_patch!+1
            set version_string=!version_major!.!version_minor!.!version_patch_plus_one!-dev.!version_commit_distance!.!version_is_dirty!
            
            if not "!version_is_dirty!"=="0" (
                set version_build_metadata=%%~V.dirty.!version_build_metadata!
            ) else (
                set version_build_metadata=%%~V.!version_build_metadata!
            )
        )
    )
) else (
    echo Git is not installed. Unable to determine version information.

    set version_major=0
    set version_minor=0
    set version_patch=0
    set version_commit_distance=0
    set version_is_dirty=0
    set version_string=!version_major!.!version_minor!.!version_patch!
    set version_build_metadata=unknown.!version_build_metadata!
)

if not "%version_build_metadata%"=="" (
    set version_string=%version_string%+%version_build_metadata:~0,-1%
)

set /a version_struct_flags=(%version_commit_distance%*16)+(%version_is_dirty%*1)

set define_version_major=#define GIT_VERSION_MAJOR %version_major%
set define_version_minor=#define GIT_VERSION_MINOR %version_minor%
set define_version_patch=#define GIT_VERSION_PATCH %version_patch%
set define_version_commit_distance=#define GIT_VERSION_COMMIT_DISTANCE %version_commit_distance%
set define_version_is_dirty=#define GIT_VERSION_IS_DIRTY %version_is_dirty%
set define_version_struct=#define GIT_VERSION_STRUCT %version_major%,%version_minor%,%version_patch%,%version_struct_flags%
set define_version_string=#define GIT_VERSION_STRING "%version_string%"

echo %define_version_major%
echo %define_version_minor%
echo %define_version_patch%
echo %define_version_commit_distance%
echo %define_version_is_dirty%
echo %define_version_struct%
echo %define_version_string%

echo %define_version_major%             >  %output_dir%\GitVersionInfo.h
echo %define_version_minor%             >> %output_dir%\GitVersionInfo.h
echo %define_version_patch%             >> %output_dir%\GitVersionInfo.h
echo %define_version_commit_distance%   >> %output_dir%\GitVersionInfo.h
echo %define_version_is_dirty%          >> %output_dir%\GitVersionInfo.h
echo %define_version_struct%            >> %output_dir%\GitVersionInfo.h
echo %define_version_string%            >> %output_dir%\GitVersionInfo.h
