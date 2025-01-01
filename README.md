# Xidi

Xidi improves the gameplay experience when using modern XInput-based controllers (such as Xbox controllers) with older games that use DirectInput or WinMM to communicate with game controllers. In more technical terms, Xidi provides both DirectInput and WinMM interfaces for games to use and communicates with XInput-based game controllers natively using XInput, translating between the two interfaces as needed.

Xidi is implemented as a library that games should load instead of the system-supplied versions. As such, it is a very localized fix: no installation is required, and no persistent system-wide changes are made.


## Key Features

- Fixes issues encountered in older games, such as broken analog controls, phantom button presses, or complete failure to commmunicate with the controller. Without Xidi these issues can come up in DirectInput-based or WinMM-based games when used with an XInput controller.

- Enables customization of game controller behavior, including simulating keyboard key presses. This can help make controls more intuitive and bring full controller support to games that only implement partial controller support.

- Allows controllers to be changed while a game is running. Older games do not normally support this, but with Xidi controllers can be plugged in, unplugged, and swapped seamlessly during gameplay. Without Xidi this would require exiting and restarting the game.


## Limitations

Xidi is not useful if:

- A game already uses the XInput API to communicate with controllers. These games would not benefit from Xidi.

- The problem arises with controllers that are not XInput-based controllers. Xidi will not communicate with non-XInput controllers.

- The problem arises from an older non-XInput controller being used with an XInput-based game. This is the inverse of the problem Xidi solves, for which solution like the [Xbox 360 Controller Emulator](https://www.x360ce.com/) is needed.


## Further Reading

See the [Wiki](https://github.com/samuelgr/Xidi/wiki) for complete documentation.
