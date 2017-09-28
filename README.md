# Xidi

Xidi provides a consistent interface between modern XInput-based controllers (i.e. Xbox-type controllers) and older games that use DirectInput or WinMM to communicate with game controllers.  While many XInput-based controllers are also exposed via DirectInput and WinMM and can be used in older games, using Xidi can lead to an improved gameplay experience.


## Features

- Ever experienced broken analog controls, phantom button presses, or complete failure to detect controller input using an Xbox-type controller with an older game?  Xidi can fix that.

- Change controllers while a game is running.  Older games do not normally support this, but with Xidi controllers can be plugged in, unplugged, and swapped seamlessly during gameplay.

- Forgot to plug in the controller before starting the game?  No problem, just plug it in after the game is running and it will be recognized automatically.  Without Xidi, it would be necessary to quit the game, plug in the controller, and re-launch.


## When to Use Xidi

Xidi is useful for improving the gameplay experience of older games with new XInput-based (i.e. Xbox-type) controllers.  The game must natively support controller input using either DirectInput or WinMM.


## When Not to Use Xidi

Xidi is not useful if:

- A game does not natively support controller input. In these situations, a solution is needed to map controller input to keyboard or mouse buttons, which Xidi does not support.

- A game already speaks XInput to controllers. These games are modern enough to support Xbox-type controllers natively.

- The controllers intended for use with a particular game are not XInput-based controllers.  Xidi will not communicate with non-XInput controllers.

- The problem arises from an older non-XInput controller being used with an XInput-based game.  This is the inverse of the problem Xidi solves, for which solution like the [Xbox 360 Controller Emulator](https://www.x360ce.com/) is needed.


# Navigation

The remainder of this document is organized as follows.

- [For Users](#for-users)
    - [Getting Started](#getting-started)
    - [Configuring Xidi](#configuring-xidi)
    - [Questions and Answers](#questions-and-answers)
- [For Developers](#for-developers)
    - [Building Xidi](#building-xidi)
    - [Source Code](#source-code)
   

# For Users

This section is intended for users who simply wish to download and use Xidi.  Topics include how to download, configure, and use Xidi.


## Getting Started

1. Ensure the [Visual C++ 2015 Runtime](https://www.microsoft.com/en-us/download/details.aspx?id=48145) is installed.  Xidi is linked against this runtime and will not work without it.  If running a 64-bit operating system, install both the x86 and the x64 versions of this runtime, otherwise install just the x86 version.

1. Download the latest release of Xidi.

1. Depending on how the game in question communicates with its controllers, place one of the supplied DLL files into the same directory as the game executable:
   - ``dinput8.dll`` for games that use DirectInput version 8
   - ``dinput.dll`` for games that use any older version of DirectInput
   - ``winmm.dll`` for games that use WinMM

1. Optionally supply [configuration settings](#configuring-xidi) to Xidi.

1. Run the game.


## Configuring Xidi

Text goes here.


## Questions and Answers

Text goes here.


# For Developers

This section is for developers who wish to compile Xidi from source and potentially modify it.  Topics include a compilation guide, a description of the organization of its source code, and an overview of the concepts that underlie its implementation.


## Building Xidi

Text goes here.


## Source Code

Source code documentation is available and can be built using Doxygen.
The remainder of this section provides an overview of how the source code is organized and, at a high level, how Xidi is implemented.

More text goes here.
