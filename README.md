Xidi provides a consistent interface between modern XInput-based controllers (i.e. Xbox-type controllers) and older games that use DirectInput or WinMM to communicate with game controllers.  While many XInput-based controllers are also exposed via DirectInput and WinMM and can be used in older games, using Xidi can lead to an improved gameplay experience.

In many respects, Xidi may be considered the inverse of projects like the [Xbox 360 Controller Emulator](https://www.x360ce.com/).  Such projects allow XInput-based games to communicate with DirectInput-based controllers.  Xidi, on the other hand, facilitates communication between DirectInput-based or WinMM-based games and XInput-based controllers.


## Features

- Ever experienced broken analog controls, phantom button presses, or complete failure to detect controller input using an Xbox-type controller with an older game?  Xidi can fix that.

- Change controllers while a game is running.  Older games do not normally support this, but with Xidi you can plug in, unplug, and swap controllers seamlessly during gameplay.

- Did you forget to plug in the controller before starting the game?  No problem, just plug it in after the game is running and it will be recognized automatically.  Without Xidi, you would need to quit the game, plug in the controller, and re-launch.


## When to Use Xidi

Xidi is useful for improving the gameplay experience of older games with new XInput-based (i.e. Xbox-type) controllers.  The game must natively support controller input using either DirectInput or WinMM.


## When Not to Use Xidi

Xidi is not useful if:

- A game does not natively support controller input. In these situations, a solution is needed to map controller input to keyboard or mouse buttons, which Xidi does not support.

- A game already speaks XInput to controllers. These games are modern enough to support Xbox-type controllers natively.

- The controllers intended for use with a particular game are not XInput-based controllers.  Xidi will not communicate with non-XInput controllers.

- The problem arises from an older non-XInput controller being used with an XInput-based game.  For this, a solution like the [Xbox 360 Controller Emulator](https://www.x360ce.com/) is needed.



# Getting Started

1. Ensure the [Visual C++ 2015 Runtime](https://www.microsoft.com/en-us/download/details.aspx?id=48145) is installed.  Xidi is linked against this runtime and will not work without it.  If running a 64-bit operating system, install both the x86 and the x64 versions of this runtime, otherwise install just the x86 version.

2. Download the latest release of Xidi.

3. Depending on how the game in question communicates with its controllers, place one of the supplied DLL files into the same directory as the game executable:  ``dinput8.dll`` for games that use DirectInput version 8, ``dinput.dll`` for games that use any older version of DirectInput, or ``winmm.dll`` for games that use WinMM.  If unsure, it is safe to place all three files into the game executable directory.

4. Optionally supply configuration settings to Xidi.

5. Run the game.
