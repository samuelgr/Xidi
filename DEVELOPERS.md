This documentation is for developers who wish to compile Xidi from source and potentially modify it.  Topics include a compilation guide, a description of the organization of its source code, and an overview of the concepts that underlie its implementation.


<h2 id="fordevelopers-buildingxidi">Building Xidi</h2>

The build system Xidi uses is based on Microsoft Visual Studio 2015 Community Edition.  To build Xidi, simply open the supplied Visual Studio solution file and build from the graphical interface.  One Visual C++ project exists for each version of Xidi (`dinput.dll`, `dinput8.dll`, and `winmm.dll`), and each supports building both 32-bit and 64-bit versions of Xidi.

Debug and Release configurations will respectively produce debug (checked) and optimized (unchecked) versions of each library.  A third configuration, TestApp, produces an executable that is designed as a standalone test suite for Xidi's functionality.  This application runs through a series of API tests designed to ensure compliance with documented API specifications for WinMM and DirectInput.  It includes an interactive controller test mode at the end, in which buttons on an attached XInput controller can be pressed and the results reflected on screen.


<h2 id="fordevelopers-sourcecode">Source Code</h2>

Source code documentation is available and can be built using Doxygen.
The remainder of this section provides an overview of how the source code is organized and, at a high level, how Xidi is implemented.

More text goes here.
