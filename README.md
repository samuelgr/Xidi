# Xidi

Xidi improves the gameplay experience when using modern XInput-based controllers (such as Xbox controllers) with older games that use DirectInput or WinMM to communicate with game controllers. In more technical terms, Xidi provides both DirectInput and WinMM interfaces for games to use and communicates with XInput-based game controllers natively using XInput, translating between the two interfaces as needed.

Xidi is implemented as a library that games should load instead of the system-supplied versions. As such, it is a very localized fix: no installation is required, and no persistent system-wide changes are made.


## Key Features

- Fixes issues encountered in older games, such as broken analog controls, phantom button presses, or complete failure to commmunicate with the controller. Without Xidi these issues can come up in DirectInput-based or WinMM-based games when used with an XInput controller.

- Allows controllers to be changed while a game is running. Older games do not normally support this, but with Xidi controllers can be plugged in, unplugged, and swapped seamlessly during gameplay. Without Xidi this would require exiting and restarting the game.


## Limitations

Xidi is not useful if:

- A game does not natively support controller input. In these situations, a solution is needed to map controller input to keyboard or mouse buttons, which Xidi does not support.

- A game already speaks XInput to controllers. These games are modern enough to support Xbox-type controllers natively.

- The problem arises with controllers that are not XInput-based controllers. Xidi will not communicate with non-XInput controllers.

- The problem arises from an older non-XInput controller being used with an XInput-based game. This is the inverse of the problem Xidi solves, for which solution like the [Xbox 360 Controller Emulator](https://www.x360ce.com/) is needed.


# Navigation

The remainder of this document is organized as follows.

- [Getting Started](#getting-started)
- [What to Expect in a Game](#what-to-expect-in-a-game)
- [Configuring Xidi](#configuring-xidi)
   - [Mapper](#mapper)
   - [Log](#log)
   - [Import](#import)
- [Mapping Controller Buttons and Axes](#mapping-controller-buttons-and-axes)
- [Questions and Answers](#questions-and-answers)
   

# Getting Started

1. Ensure the combined [Visual C++ 2015, 2017, and 2019 Runtime](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) is installed. Xidi is linked against this runtime and will not work without it. If running a 64-bit operating system, install both the x86 and the x64 versions of this runtime, otherwise install just the x86 version.

1. Download the latest release of Xidi.

1. Install one of the [forms of Xidi](#forms-of-xidi) into the same directory as the game executable.

1. Optionally supply [configuration settings](#configuring-xidi) to Xidi.

1. Run the game.


## Forms of Xidi

Xidi is available in multiple forms, each of which caters to a particular way in which games might communicate with game controllers. The following subsections describe each form of Xidi and how to install it.


### DirectInput

Many games use the DirectInput API to communicate with various input devices, including game controllers. For such games, place either `dinput8.dll` or `dinput.dll` into the same directory as the game executable. The former is for games that use version 8 of DirectInput, and the latter is for games that use any older version.


### WinMM

Some games make use of the legacy joystick API offered by the Windows multimedia library. To use Xidi with these games, place `winmm.dll` into the same directory as the game executable.


### HookModule

**It is not common for games to need the HookModule form of Xidi.**

Some games and mods bypass the loading mechanism upon which Xidi's other forms rely.  The result of doing so is that even following the directions above is insufficient for Xidi to work.  Both DirectInput and WinMM forms of Xidi are potentially affected.

Certain games that use DirectInput do not directly load the DLL but rather use a Windows subsystem known as Component Object Model (COM) to request that the system figure out how to access DirectInput functionality. This makes use of information in the system registry which ends up pointing to the system-installed version of DirectInput.  For WinMM, this problem can arise when the DLL search path is programmatically altered or if the WinMM DLL is loaded using by some other mechanism like [API sets](https://docs.microsoft.com/en-us/windows/win32/apiindex/windows-apisets).  In both cases, the HookModule form of Xidi intercepts invocations of the system-supplied functions and redirects them to Xidi.

Games that do not work with Xidi's other forms alone require the HookModule form of Xidi, which is a hook module intended to be loaded by [Hookshot](https://github.com/samuelgr/Hookshot). To install the HookModule form of Xidi:
1. Place either `dinput8.dll`, `dinput.dll`, or `winmm.dll` into the same directory as the game executable, as usual for the other forms of Xidi.
1. Additionally place `Xidi.HookModule.XX.dll` into the same directory as the game executable.
1. Download the [latest release of Hookshot](https://github.com/samuelgr/Hookshot/releases), version 1.2.0 or higher. Extract `Hookshot.XX.exe` and `Hookshot.XX.dll` into the same directory as the game executable.
1. Run the game with Hookshot. The easiest way to do this is using the Hookshot Launcher, as follows, but there are other ways that are described in Hookshot's documentation. To use Hookshot Launcher:
    1. Rename the game executable by adding the text `_HookshotLauncher_` to the beginning. For example, `Game.exe` would be renamed to `_HookshotLauncher_Game.exe`.
    1. From the downloaded Hookshot release, extract `HookshotLauncher.XX.exe` to the same directory as the game executable, then rename it to the original name of the game executable. Following the example above, this would mean renaming `HookshotLauncher.XX.exe` to `Game.exe`.
    1. Run the game as normal. Hookshot Launcher will take care of ensuring Hookshot loads the game correctly.

In all of the steps above, note that `XX` is either `32` or `64` depending on whether the game executable is 32-bit or 64-bit.


# What to Expect in a Game

XInput identifies controllers by player number rather than by specific controller device (i.e. player 1, player 2, and so on), and Xidi therefore does the same thing. The system determines the assignment of controller device to player automatically, and these assignments may change as controller devices are plugged into and unplugged from the system. Some controllers such as the Xbox 360 controller indicate the assigned player number directly on the device, so it is easy to determine which controller is assigned to which player number.

If a game supports explicitly specifying a controller device to use, then Xidi virtual controller devices with names similar to "Xidi Virtual Controller 1" will be visible during the configuration process. Each such controller maps to the XInput controller of the shown player number. These Xidi-supplied controllers would need to be selected during configuration in order for the corresponding XInput controllers to be used in the game. Any non-XInput controllers would still be available for selection during the configuration process, but all XInput-based controllers would be exposed only through the Xidi-supplied controllers.

On the other hand, if a game does not allow a specific controller device to be set during configuration, then generally whichever controller is assigned to player 1 would automatically start working in the game. Games that support multiple controllers would additionally work with other controllers in ascending player number order. However, note that there are some caveats that apply to games of this type. For more information, see [questions and answers](#questions-and-answers).

A game that binds to Xidi virtual controller devices, either automatically or via explicit configuration, will support seamless controller changes. If no controllers are plugged in when the game starts, simply plugging in an XInput controller will cause it to start working. The same holds for adding additional players to a running game or for swapping controllers already in use.


# Configuring Xidi

Xidi is designed to require minimal, if any, configuration. Defaults were carefully selected to maximize the probability of compatibility. Nevertheless, it can be configured by placing a text file named `Xidi.ini` into the same directory as the running form of Xidi. This file is optional; if not present, Xidi simply uses its default settings.

An example configuration file is shown below, containing default values for all available settings. The remainder of this subsection describes each setting and what can be supplied as a value.

```ini
[Mapper]
Type = StandardGamepad

[Log]
Enabled = no
Level = 1

[Import]
dinput.dll = C:\Windows\system32\dinput.dll
dinput8.dll = C:\Windows\system32\dinput8.dll
winmm.dll = C:\Windows\system32\winmm.dll
```


## Mapper

This section controls the mapping scheme Xidi uses when mapping between XInput and DirectInput controller elements.

- **Type** specifies the [type of mapper](#mapping-controller-buttons-and-axes) that Xidi should use. Supported values are the names of each included mapper type.


## Log

This section controls Xidi's logging output. Logging should generally be disabled unless compatibility issues are discovered.

Any log files Xidi produces are placed on the current user's desktop and are named to specify both the executable and the specific version of Xidi that was loaded (`dinput.dll`, `dinput8.dll`, or `winmm.dll`).

- **Enabled** specifies whether or not Xidi should produce a log during game execution. Supported values are `yes` and `no`.

- **Level** specifies the verbosity of logging. Supported values range from 1 (show only errors that will affect behavior) to 4 (show detailed debugging logs).


## Import

This section provides advanced functionality unlikely to be needed by most users. Unless there is a specific need for this feature, its use should be avoided.

The settings in this section override Xidi's default import library paths. Xidi relies on functions provided by the system-supplied version of each library it emulates (`dinput.dll`, `dinput8.dll`, or `winmm.dll`), so by default it loads the system-supplied version. The example above shows a hard-coded default path, but in reality Xidi determines the system path dynamically.

In some situations, it may be desirable for Xidi to use functions provided by a different library file instead. If this is the case, then values in this section should be specified. Paths can be relative to the directory containing the executable file or absolute.

- **dinput.dll** specifies the path of the DLL file that Xidi should load instead of the system-supplied `dinput.dll` file.

- **dinput8.dll** specifies the path of the DLL file that Xidi should load instead of the system-supplied `dinput8.dll` file.

- **winmm.dll** specifies the path of the DLL file that Xidi should load instead of the system-supplied `winmm.dll` file.


# Mapping Controller Buttons and Axes

An XInput-based controller follows the controller layout of an Xbox controller: buttons have names (A, B, X, Y, and so on), and analog axes are identified directly (left stick, right stick, LT, RT). Games that natively support XInput can simply refer to controller components by name, such as by saying "press A to jump" or "the right stick controls the camera."

DirectInput, on the other hand, supports every possible form factor for a game controller: gamepads, joysticks and steering wheels, to name a few. It does not have a defined standard layout for controllers. Games generally refer to buttons by number (button 1, button 2, and so on) and analog axes by letter (X axis, Y axis, and so on), and it is up to the manufacturer of the controller to determine which physical button or axis corresponds to each number or letter, respectively. Xidi therefore implements a mapping of Xbox controller components to DirectInput controller components.

Most buttons and axes can be mapped directly from XInput to DirectInput, but the LT and RT triggers present a complication. DirectInput axes assume that the neutral (i.e. unpressed) position is at the center. This works for analog sticks and joysticks because they return to the center position when the user is not actively pushing them in a particular direction. However, LT and RT do not behave this way: the neutral position is at an extreme end, so mapping them directly to a DirectInput axis will cause games to break unless they are aware of this specific behavior or they ignore LT and RT completely.

Many games make assumptions about the controller layout, and Xidi cannot automatically determine these assumptions. Xidi therefore includes several types of mappers and allows the specific mapper that is used to be configured. While the default, `StandardGamepad`, is expected to work well with a wide variety of games, there are undoubtedly situations in which it is unsuitable. Specifically, Xidi provides the following mappers.

- **StandardGamepad** emulates old gamepads that resemble Sony PlayStation controllers, such as the Logitech RumblePad 2. As was common for these controllers, the right stick maps to the Z and Z-rotation axes. LT and RT are mapped to digital buttons; all analog functionality is removed, but the controls are usable.

- **DigitalGamepad** emulates old digital-only gamepads like the Gravis GamePad Pro. All axes are digital (i.e. no analog functionality, either they are pressed to the extreme or not pressed at all). Both left stick and d-pad contribute to X and Y axes. While controllers of this type typically only had two axes, the right stick nonetheless offers Z and Z-rotation axes in digital form.

- **ExtendedGamepad** is similar to StandardGamepad, except that the analog functionality of LT and RT is preserved. The X-rotation and Y-rotation axes were selected to retain the mapping of the right stick to the Z and Z-rotation axes.

- **XInputNative** mirrors the previous behavior of Windows 10 when an Xbox One controller was connected. The right stick maps to X-rotation and Y-rotation, and the triggers individually map to Z and Z-rotation. This configuration broke many older games but is presented as an option to support those that require it.

- **XInputSharedTriggers** mirrors the current behavior of Windows 10 when an Xbox One or Xbox 360 controller is connected. The right stick maps to X-rotation and Y-rotation, but the triggers share the Z axis. This preserves the analog functionality of the triggers but prevents them from being used independently.

Precise button and axis mappings are as below.

|                         | StandardGamepad   | DigitalGamepad             | ExtendedGamepad   | XInputNative      | XInputSharedTriggers   |
| ----------------------- | ----------------- | -----------------          | ----------------- | ----------------- | ---------------------- |
| Left Stick, Horizontal  | X axis            | X axis (digital)           | X axis            | X axis            | X axis                 |
| Left Stick, Vertical    | Y axis            | Y axis (digital)           | Y axis            | Y axis            | Y axis                 |
| Right Stick, Horizontal | Z axis            | Z axis (digital)           | Z axis            | X-rotation axis   | X-rotation axis        |
| Right Stick, Vertical   | Z-rotation axis   | Z-rotation axis (digital)  | Z-rotation axis   | Y-rotation axis   | Y-rotation axis        |
| D-Pad                   | Point-of-view hat | X and Y axes (digital)     | Point-of-view hat | Point-of-view hat | Point-of-view hat      |
| A                       | Button 1          | Button 1                   | Button 1          | Button 1          | Button 1               |
| B                       | Button 2          | Button 2                   | Button 2          | Button 2          | Button 2               |
| X                       | Button 3          | Button 3                   | Button 3          | Button 3          | Button 3               |
| Y                       | Button 4          | Button 4                   | Button 4          | Button 4          | Button 4               |
| LB                      | Button 5          | Button 5                   | Button 5          | Button 5          | Button 5               |
| RB                      | Button 6          | Button 6                   | Button 6          | Button 6          | Button 6               |
| LT                      | Button 7          | Button 7                   | X-rotation axis   | Z-axis            | Z-axis (shared)        |
| RT                      | Button 8          | Button 8                   | Y-rotation axis   | Z-rotation axis   | Z-axis (shared)        |
| Back                    | Button 9          | Button 9                   | Button 7          | Button 7          | Button 7               |
| Start                   | Button 10         | Button 10                  | Button 8          | Button 8          | Button 8               |
| LS                      | Button 11         | Button 11                  | Button 9          | Button 9          | Button 9               |
| RS                      | Button 12         | Button 12                  | Button 10         | Button 10         | Button 10              |


# Questions and Answers

#### Which specific controllers does Xidi support?

Any controller that supports Windows and XInput can be used with Xidi. This includes non-Microsoft controllers, wireless controllers, wired controllers, and so on. That being said, only Xbox 360 and Xbox One controllers have been specifically tested. Any issues with other controllers are likely indicative of bugs in Xidi.

#### Can I use Xidi with multiple controllers?

Yes. Xidi supports as many XInput-based controllers as does the XInput API itself. The current limit is 4 controllers.

#### What if I have more than the maximum supported number of XInput controllers?

The only XInput controllers that Xidi would expose to the game are those that are assigned XInput player numbers. Any controllers above the limit would be unavailable for use in the game; they cannot be accessed, even by using DirectInput.

#### Does Xidi support using XInput controllers and non-XInput controllers together?

Yes. Xidi does not interfere in any way with the functionality of non-XInput controllers. A game that attempts to enumerate DirectInput or WinMM controllers would see Xidi virtual devices in addition to any attached DirectInput or WinMM controllers that do not support XInput.

#### If I have non-XInput controllers attached and I start a game that does not let me pick a specific controller, what happens?

Typically a game would enumerate game controllers when it starts and would simply bind to whichever controller it sees first. However, Xidi does take this scenario into account and controls the order in which controllers are presented to games during enumeration. Xidi uses different rules depending on which version is in use (DirectInput or WinMM). Before presenting enumerated controllers to the game, Xidi performs its own enumeration and applies these rules internally.

The DirectInput version uses the following rules to determine enumeration order. Note that DirectInput supports many device types other than game controllers, such as keyboards and mice, so applications that request enumeration can specify one or more device classes to narrow down the results.

- If the game requests enumeration of a class of devices that includes game controllers, then game controllers always come first. The rules that follow determine the order in which game controllers are presented.

- If none of the game controllers physically attached to the system support XInput, then non-XInput devices are presented first, followed by Xidi virtual devices.

- If at least one game controller physically attached to the system supports XInput, then Xidi virtual devices are presented first, followed by non-XInput devices.

- If no game controllers are physically attached to the system, then only Xidi virtual devices are presented.

The WinMM version uses the following rules to determine enumeration order. Note that "preferred game controller" refers to whatever controller is specified in the "Game Controllers" system control panel under the "Advanced" options.

- If the preferred game controller is specified and physically present but does not support XInput, then non-XInput devices are enumerated first, followed by Xidi virtual devices.

- If the preferred game controller supports XInput or is either unspecified or physically absent, then Xidi virtual devices are enumerated first, followed by non-XInput devices.

#### How do I add a controller or change to a different controller while the game is running?

Simply connect or disconnect controllers as needed. Xidi's use of virtual devices means that the game is bound to an XInput player number, not a physical device. While the corresponding controller is disconnected, the game is informed that there are no buttons or axes pressed (i.e. the controller is completely neutral).

#### DirectInput supports force feedback. What about Xidi?

Xidi does not currently support force feedback, but this feature may be added in a future version.
