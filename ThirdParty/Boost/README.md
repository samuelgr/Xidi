# Boost Third-Party Package

With the goal of limiting its size, this package includes only those Boost modules needed by the application along with their dependencies.


## Included Modules

This application uses the following Boost modules.
- circular_buffer


## Recreation Instructions

To recreate a Boost package consisting of only selected modules from a Boost source package:

1. [Download](https://www.boost.org/users/download/) a source package.
1. Extract the source package anywhere. The remaining steps assume the source package is extracted to `C:\boost`.
1. Bootstrap the Boost build system. At a command prompt:
   - `c:` 
   - `cd \boost\boost*`
   - `bootstrap.bat`
1. Build the `bcp` tool. Continuing at the same command prompt as in the previous step:
   - `b2 tools\bcp`
1. Use the `bcp` tool to generate a report of the licenses associated with the desired Boost modules. This file need not be included in the package but is useful as a licensing reference. Continuing at the same command prompt as in the previous step:
   - `dist\bin\bcp.exe --report <boost-modules-list> <html-file>`
1. Use the `bcp` tool to create a package with only specific Boost modules. Continuing at the same command prompt as in the previous step:
   - `dist\bin\bcp.exe <boost-modules-list> <output-dir>`
