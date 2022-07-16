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


# Navigation

The remainder of this document is organized as follows.

- [Getting Started](#getting-started)
- [What to Expect in a Game](#what-to-expect-in-a-game)
- [Configuring Xidi](#configuring-xidi)
   - [Mapper](#mapper)
   - [Log](#log)
   - [Import](#import)
   - [CustomMapper](#custommapper)
   - [Workarounds](#workarounds)
- [Mapping Controller Buttons and Axes](#mapping-controller-buttons-and-axes)
   - [Built-In Mappers](#built-in-mappers)
   - [Custom Mappers](#custom-mappers)
     - [Defining a Custom Mapper](#defining-a-custom-mapper)
     - [Element Mappers](#element-mappers)
     - [Force Feedback Actuators](#force-feedback-actuators)
- [Questions and Answers](#questions-and-answers)
   

# Getting Started

1. Ensure the system is running Windows 10 or 11. Xidi is built to target Windows 10 or 11 and does not support older versions of Windows.

1. Ensure the [Visual C++ Runtime for Visual Studio 2022](https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) is installed. Xidi is linked against this runtime and will not work without it. If running a 64-bit operating system, install both the x86 and the x64 versions of this runtime, otherwise install just the x86 version.

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


#### Installation Steps

Games that do not work with Xidi's other forms alone might work with the HookModule form of Xidi, which is a hook module intended to be loaded by [Hookshot](https://github.com/samuelgr/Hookshot). To install the HookModule form of Xidi:
1. Place either `dinput8.dll`, `dinput.dll`, or `winmm.dll` into the same directory as the game executable, as usual for the other forms of Xidi.
1. Additionally place `Xidi.HookModule.XX.dll` into the same directory as the game executable.
1. Download [Hookshot](https://github.com/samuelgr/Hookshot/releases) version 1.2.0 or higher. Extract `Hookshot.XX.exe` and `Hookshot.XX.dll` into the same directory as the game executable.
1. Run the game with Hookshot. The easiest way to do this is using the Hookshot Launcher, as follows, but there are other ways that are described in Hookshot's documentation. To use Hookshot Launcher:
    1. Rename the game executable by adding the text `_HookshotLauncher_` to the beginning. For example, `Game.exe` would be renamed to `_HookshotLauncher_Game.exe`.
    1. From the downloaded Hookshot release, extract `HookshotLauncher.XX.exe` to the same directory as the game executable, then rename it to the original name of the game executable. Following the example above, this would mean renaming `HookshotLauncher.XX.exe` to `Game.exe`.
    1. Run the game as normal. Hookshot Launcher will take care of ensuring Hookshot loads the game correctly.

In all of the steps above, note that `XX` is either `32` or `64` depending on whether the game executable is 32-bit or 64-bit.

#### Technical Explanation

Certain games that use DirectInput do not directly load the DLL but rather use a Windows subsystem known as Component Object Model (COM) to request that the system figure out how to access DirectInput functionality. This makes use of information in the system registry which ends up pointing to the system-installed version of DirectInput.  The HookModule form of Xidi intercepts one of the key functions offered by COM to ensure that requests for DirectInput objects go through Xidi.

WinMM games typically do not themselves intentionally bypass the default library loading mechanism. Nonetheless, this problem can arise when the DLL search path is programmatically altered or if the WinMM DLL is loaded using by some other mechanism like [API sets](https://docs.microsoft.com/en-us/windows/win32/apiindex/windows-apisets).  In both cases, the HookModule form of Xidi intercepts invocations of the system-supplied functions and redirects them to Xidi.


# What to Expect in a Game

XInput identifies controllers by player number rather than by specific controller device (i.e. player 1, player 2, and so on), and Xidi therefore does the same thing. The system determines the assignment of controller device to player automatically, and these assignments may change as controller devices are plugged into and unplugged from the system. Some controllers such as the Xbox 360 controller indicate the assigned player number directly on the device, so it is easy to determine which controller is assigned to which player number.

If a game supports explicitly specifying a controller device to use, then Xidi virtual controller devices with names similar to "Xidi Virtual Controller 1" will be visible during the configuration process. Each such controller maps to the XInput controller of the shown player number. These Xidi-supplied controllers would need to be selected during configuration in order for the corresponding XInput controllers to be used in the game. Any non-XInput controllers would still be available for selection during the configuration process, but all XInput-based controllers would be exposed only through the Xidi-supplied controllers.

On the other hand, if a game does not allow a specific controller device to be set during configuration, then generally whichever controller is assigned to player 1 would automatically start working in the game. Games that support multiple controllers would additionally work with other controllers in ascending player number order. However, note that there are some caveats that apply to games of this type. For more information, see [questions and answers](#questions-and-answers).

A game that binds to Xidi virtual controller devices, either automatically or via explicit configuration, will support seamless controller changes. If no controllers are plugged in when the game starts, simply plugging in an XInput controller will cause it to start working. The same holds for adding additional players to a running game or for swapping controllers already in use.


# Configuring Xidi

Xidi is designed to require minimal, if any, configuration. Defaults were carefully selected to maximize the probability of compatibility. Nevertheless, it can be configured by placing a text file named `Xidi.ini` into the same directory as the running form of Xidi. This file is optional; if not present, Xidi simply uses its default settings.

Xidi will display a warning message box and automatically enable logging if it detects a configuration file error on application start-up. Consult the log file that Xidi places on the desktop for the details of any errors.

An example configuration file is shown below, containing default values for all available settings. The remainder of this subsection describes each setting and what can be supplied as a value.

```ini
[Mapper]
Type                = StandardGamepad
Type.1              = StandardGamepad
Type.2              = StandardGamepad
Type.3              = StandardGamepad
Type.4              = StandardGamepad

[Log]
Enabled             = no
Level               = 1

[Import]
dinput.dll          = C:\Windows\system32\dinput.dll
dinput8.dll         = C:\Windows\system32\dinput8.dll
winmm.dll           = C:\Windows\system32\winmm.dll

[CustomMapper]
; This section does not exist by default.
```


## Mapper

This section controls the mapping scheme Xidi uses when mapping between XInput and DirectInput controller elements.

- **Type** specifies the [type of mapper](#mapping-controller-buttons-and-axes) that Xidi should for all controllers by default. Supported values are the names of any mapper, [built-in](#built-in-mappers) or [custom](#custom-mappers).
- **Type.1** specifies the type of mapper that Xidi should use for controller 1, overriding the default.
- **Type.2** specifies the type of mapper that Xidi should use for controller 2, overriding the default.
- **Type.3** specifies the type of mapper that Xidi should use for controller 3, overriding the default.
- **Type.4** specifies the type of mapper that Xidi should use for controller 4, overriding the default.


## Log

This section controls Xidi's logging output. Logging should generally be disabled unless issues are discovered.

Any log files Xidi produces are placed on the current user's desktop and are named to specify both the executable and the specific form of Xidi that was loaded.

- **Enabled** specifies whether or not Xidi should produce a log during game execution. Supported values are `yes` and `no`.

- **Level** specifies the verbosity of logging. Supported values range from 1 (show only errors that will affect behavior) to 4 (show detailed debugging logs).


## Import

**It is not common for there to be a need to modify the settings in this section.**

This section provides advanced functionality unlikely to be needed by most users. Unless there is a specific need for this feature, its use should be avoided.

The settings in this section override Xidi's default import library paths. Xidi relies on functions provided by the system-supplied version of each library it emulates (`dinput.dll`, `dinput8.dll`, or `winmm.dll`), so by default it loads the system-supplied version. The example above shows a hard-coded default path, but in reality Xidi determines the system path dynamically.

In some situations, it may be desirable for Xidi to use functions provided by a different library file instead. If this is the case, then values in this section should be specified. Paths can be relative to the directory containing the executable file or absolute.

- **dinput.dll** specifies the path of the DLL file that Xidi should load instead of the system-supplied `dinput.dll` file.

- **dinput8.dll** specifies the path of the DLL file that Xidi should load instead of the system-supplied `dinput8.dll` file.

- **winmm.dll** specifies the path of the DLL file that Xidi should load instead of the system-supplied `winmm.dll` file.


## CustomMapper

This section is used to define a custom mapper type that specifies how Xidi should translate XInput controller elements to virtual controller elements and keyboard keys. See [Custom Mappers](#custom-mappers) for more information.


## Workarounds *(unreleased)*

**It is not common for there to be a need to modify the settings in this section.**

This section is used to modify the behavior of Xidi in ways that are not generally required by the DirectInput or WinMM APIs but are necessary to work around bugs in the original implementation of certain applications. Unless an application requires any of the available workarounds, the use of this section should be avoided.

- **MaxVirtualControllerCount** limits the number of Xidi virtual controllers that are made available to the application. By default, Xidi makes available as many controllers as are possible under the XInput API, but some applications are implemented incorrectly such that they cannot properly distinguish between inputs from multiple controllers.


# Mapping Controller Buttons and Axes

An XInput-based controller follows the controller layout of an Xbox controller: buttons have names (A, B, X, Y, and so on), and analog axes are identified directly (left stick, right stick, LT, and RT). Games that natively support XInput can simply refer to controller components by name, such as by saying "press A to jump" or "the right stick controls the camera."

DirectInput, on the other hand, supports every possible form factor for a game controller: gamepads, joysticks and steering wheels, to name a few. It does not have a defined standard layout for controllers. Games generally refer to buttons by number (button 1, button 2, and so on) and analog axes by letter (X axis, Y axis, and so on), and it is up to the manufacturer of the controller to determine which physical button or axis corresponds to each number or letter, respectively. Xidi therefore implements a mapping of Xbox controller components to DirectInput controller components.

Most buttons and axes can be mapped directly from XInput to DirectInput, but the LT and RT triggers present a complication. DirectInput axes assume that the neutral (i.e. unpressed) position is at the center. This works for analog sticks and joysticks because they return to the center position when the user is not actively pushing them in a particular direction. However, LT and RT do not behave this way: the neutral position is at an extreme end, so mapping them directly to a DirectInput axis will cause games to break unless they are aware of this specific behavior or they ignore LT and RT completely.

Many games make assumptions about the controller layout, and Xidi cannot automatically determine these assumptions. Xidi therefore allows XInput controller mapping behavior to be customized. It includes several built-in types of mappers and supports custom mappers being defined in configuration files.


## Built-In Mappers

Xidi defines several built-in mapper types, as listed below.

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


## Custom Mappers

Recognizing that the built-in mapper selection might not be optimal for all games, Xidi allows mapper behavior to be customized using a configuration file. A small collection of pre-made custom mapper configurations is available in the [XidiGameConfigurations](https://www.github.com/samuelgr/XidiGameConfigurations) repository.

The simplest way to create a custom mapper is to include a configuration file section named "CustomMapper" and specify "Custom" as the desired type of mapper. An example is shown below.

```ini
[Mapper]
Type                = Custom

[CustomMapper]
; Custom mapper settings go here.
```

It is also possible to define multiple custom mappers and give each its own name. This is done by using section names of the form "CustomMapper:*name*" and referencing *name* when specifying a mapper type. For example, to assign different custom mappers to each controller, the following configuration file could be used.

```ini
[Mapper]
Type.1              = FancyMapper1
Type.2              = Game2
Type.3              = Three
Type.4              = 4thMapper

[CustomMapper:FancyMapper1]
; Custom mapper settings for "FancyMapper1" go here.

[CustomMapper:Game2]
; Custom mapper settings for "Game2" go here.

[CustomMapper:Three]
; Custom mapper settings for "Three" go here.

[CustomMapper:4thMapper]
; Custom mapper settings for "4thMapper" go here.
```

Custom mappers named this way must:
  - Have unique names. No two custom mappers may share the same name.
  - Not be named "Custom" as this would clash with the default name of a custom mapper.
  - Not have the same name as any of the built-in mappers.

Xidi will display a warning message box if it detects an error with how a custom mapper is defined. To see the details, ensure logging is turned on and consult the log file Xidi places on the desktop. Unlike for configuration file errors, Xidi will not automatically turn on logging in the event of a custom mapper definition error.

The subsections that follow describe what each "CustomMapper" configuration section should contain in order to define a custom mapper.


### Defining a Custom Mapper

Configuration file sections that define custom mappers contain two types of settings: top-level settings and element mappers.

Currently the only available *top-level setting* is "Template" whose value is the name of any other mapper, be it built-in or custom. This has the effect of copying all of the element mappers from the template mapper to use as a starting point for the custom mapper. Individual element mappers can still be specified, and these are applied as modifications to the template. Use of a template is optional; in the absence of a template the custom mapper is built from scratch. Ordering of custom mapper definitions within the configuration file is not important. In the preceding example, "FancyMapper1" is allowed to use any of "Game2," "Three," and "4thMapper" as a template even though their definitions appear later in the configuration file. However, self-references and circular references are forbidden, and Xidi will flag an error if any such issues are detected.

An *element mapper* defines how Xidi should process input from a specific XInput controller element. For example, an element mapper assigned to the A button might specify that input should be routed to button 1 on a virtual controller. At the same time, another element mapper assigned to trigger LT might specify that input should be routed to the RotZ axis on the virtual controller.

The configuration below shows how to create a custom mapper that behaves the same way as StandardGamepad but modifies the button assignments of A, B, X, and Y. Ordering of the lines within each section is not important.

```ini
[CustomMapper:ModifiedStandardGamepad]

; Imports all of the element mappers from StandardGamepad.
; This line is allowed to appear anywhere in the section, including between or after the element mappers below.
Template            = StandardGamepad

; These changes are applied on top of the template.
ButtonA             = Button(4)
ButtonB             = Button(3)
ButtonX             = Button(2)
ButtonY             = Button(1)
```

The above configuration is equivalent to the below configuration, which does the same thing but without the use of a template. Shown are all of the supported XInput controller elements to which an element mapper can be assigned. If an XInput controller element is not assigned an element mapper then all input from it is ignored.

```ini
[CustomMapper:ModifiedStandardGamepadNoTemplate]

; Because there is no template specified, every element mapper needs to be defined explicitly.
; Element mappers below are taken from StandardGamepad documentation.

StickLeftX          = Axis(X)
StickLeftY          = Axis(Y)
StickRightX         = Axis(Z)
StickRightY         = Axis(RotZ)

DpadUp              = Pov(Up)
DpadDown            = Pov(Down)
DpadLeft            = Pov(Left)
DpadRight           = Pov(Right)

ButtonLB            = Button(5)
ButtonRB            = Button(6) 
TriggerLT           = Button(7)
TriggerRT           = Button(8)
ButtonBack          = Button(9)
ButtonStart         = Button(10)
ButtonLS            = Button(11)
ButtonRS            = Button(12)

; These are different from StandardGamepad, per the original purpose of this example.

ButtonA             = Button(4)
ButtonB             = Button(3)
ButtonX             = Button(2)
ButtonY             = Button(1)
```

It is permissible for multiple element mappers to be linked to the same virtual controller element. Xidi intelligently combines the multiple inputs into a single coherent output as follows.
 - Axis output is determined by summation of contributions from element mappers.
 - Button output is determined by the logical-or of contributions from element mappers. In other words, if any element mapper says the button is pressed, Xidi reports it as pressed.
 - POV hat output is determined by proper direction combination.
   - Opposing contributions (for example, simultaneous up and down) are cancelled out.
   - Orthogonal contributions (for example, simultaneous left and up) are combined into a diagonal output.

Custom mappers theoretically can define virtual controllers of arbitrary capabilities (i.e. which axes are present, how many buttons are present, and whether or not a POV hat exists). Xidi imposes certain limits to simplify its own implementation and to accomodate expectations of both the DirectInput and WinMM APIs. Specifically, the following limits exist.
- Axes
  - Minimum: X axis and Y axis must both be present.
    - If no element mapper is linked to one of these axes then the application's view is that the corresponding axis is present but always held in a neutral position.
  - Maximum: X, Y, Z, RotX, RotY, and RotZ axes are supported.
- Buttons
  - The highest-numbered button determines the number of buttons Xidi reports to the application. Any buttons not linked to an element mapper are always held in an unpressed state.
    - For example, if a custom mapper specifies buttons 5 and 10, then Xidi reports that a total 10 buttons exist on the virtual controller, but only buttons 5 and 10 could ever possibly be pressed.
  - Minimum: 2 buttons must be present.
    - If no element mappers contribute to a virtual controller button, then Xidi will report 2 buttons being present even though neither can ever be pressed.
  - Maximum: 16 buttons are supported.
- POV hats
  - If any element mapper is linked to the POV hat, Xidi reports that the POV hat is present. Otherwise Xidi reports that it is not present.
  - Minimum: 0 POV hats exist on the virtual controller.
  - Maximum: 1 POV hat exists on the virtual controller.


### Element Mappers

Xidi provides several different types of element mappers, each of which implements a different way of translating input from an XInput controller element into virtual controller state. While some are designed with certain use cases in mind, there are no restrictions on the types of element mappers that can be linked to any specific XInput controller elements.


#### Axis and DigitalAxis

Both types of element mapper link an XInput controller element to a virtual controller axis. The difference between them is that Axis produces analog output whereas DigitalAxis maps all input to a digital output. In other words, any representable analog value can be produced by Axis, whereas DigitalAxis will produce either extreme negative, neutral, or extreme positive. This only manifests in different behavior if the element mapper is linked to an analog input source, such as a stick or a trigger.

Axis and DigitalAxis both require a parameter specifying the virtual controller axis to which to link. Supported axis names are `X`, `Y`, `Z`, `RotX`, `RotY`, and `RotZ`. A second optional parameter is additionally allowed to specify the axis direction, either `+` or `-` (alternative values `Positive` and `Negative` are also accepted).

By default all Axis and DigitalAxis element mappers are bidirectional, meaning they cover the entire range of possible axis values from extreme negative to extreme positive. Specifying a direction modifies this behavior and is primarily useful for Axis and DigitalAxis element mappers that accept input from XInput controller buttons. In bidirectional mode the axis is reported as extreme positive if the button is pressed and extreme negative if not pressed. In unidirectional mode the axis is reported as extreme in the configured direction if the button is pressed and neutral if the button is not pressed.

To see how this unidirectional configuration works in practice, below are two examples to highlight the difference.

```ini
[CustomMapper:BidirectionalAxisExample]

; Bidirectional axis example.
; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

; The A button is mapped to the X axis. Resulting behavior is likely undesirable.
; If the A button is pressed, X axis is extreme positive (i.e. all the way to the right).
; If the A button is not pressed, X axis is extreme negative (i.e. all the way to the left).
ButtonA             = Axis(X)


[CustomMapper:UnidirectionalAxisExample]

; Unidirectional axis example.
; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

; The A button is mapped to the positive direction of the X axis.
; If the A button is pressed, X axis is extreme positive (i.e. all the way to the right).
; If the A button is not pressed, X axis is neutral (i.e. centered).
ButtonA             = Axis(X, +)
```

XInputSharedTriggers, a built-in mapper, uses unidirectional Axis element mappers to implement sharing of the Z axis across both triggers. The example below shows how this would be represented in a configuration file.

```ini
[CustomMapper:XInputSharedTriggersExample]

; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

; One trigger each is assigned to a different Z axis direction, equivalent to XInputSharedTriggers behavior.
TriggerLT           = Axis(Z, +)
TriggerRT           = Axis(Z, -)
```


#### Button

A Button element mapper links an XInput controller element to a virtual controller button. It requires a single parameter specifying the button number, from 1 to 16.


#### Compound

A Compound element mapper forwards input to multiple element mappers, up to a maximum of 8. It requires one or more parameters each specifying an element mapper.

The example below shows how to use a Compound element mapper to link an XInput controller button to both a virtual controller button and a keyboard key. As a result, pressing the XInput controller button has the effect causing both the virtual controller button and the keyboard key to be pressed.

```ini
[CustomMapper:CompoundExample]

; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

; Routes input from the Start button to both button 10 on the virtual controller the Enter key on the keyboard.
; Parameter order does not matter.
ButtonStart         = Compound( Button(10), Keyboard(Enter) )
```


#### Invert

An Invert element mapper inverts whatever input it receives from its associated XInput controller element and then forwards the result to another element mapper. It requires a single parameter specifying an element mapper.

This type of element mapper is primarily useful for inverting axis values, which has the effect of swapping the positive and negative directions. The example below shows how this would be implemented in a configuration file.

```ini
[CustomMapper:InvertExample]

; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

; Inverts analog input received from the left stick.
; Normally, right direction on the analog stick is positive and left direction is negative.
; As a result of the inversion, right direction on the analog stick is negative and left direction is positive.
StickLeftX          = Invert( Axis(X) )
```

Inversion also works on triggers and buttons. Triggers follow the same basic inversion logic as axes, and buttons have their pressed and unpressed states swapped.


#### Keyboard

A keyboard element mapper links an XInput controller element to a key on the keyboard. As a result, it is not linked to any virtual controller element.

Element mappers of this type require a single parameter identifying the associated keyboard key. Keyboard keys can be identified in a few different ways, as listed below in order of precedence from top to bottom.
1. Symbolic key name, which is case-insensitive and takes the form of one of the `DIK_` enumerator names from the DirectInput [keyboard device enumeration](https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee418641(v=vs.85)), with or without the "DIK_" prefix.
   - This is the recommended way to identify a keyboard key.
1. Raw numeric scan code expressed as a single byte number in decimal, octal (prefix "0" required), or hexadecimal (prefix "0x" required).
   - Valid values are 0 to 255 (0x0 to 0xFF).
   - Scan codes that identify keys other than extended keys fall into the range 0 to 127 (0x0 to 0x7F).
   - Scan codes that identify extended keys typically use two bytes, the first of which is the prefix byte 0xE0. Instead of including a prefix byte, follow the DirectInput convention of setting the most significant bit to obtain a value from 128 to 255 (0x80 to 0xFF).

In general, the easiest way of identifying a keyboard key is by symbolic name. The complete list of supported symbolic names is contained in the `MapperParser.cpp` source code file, but as a summary the following are recognized. Note that Xidi internally maps all keyboard key identifiers to hardware scan codes. Therefore, all symbolic names represent physical key positions on a US QWERTY keyboard.

- `Up`, `Down`, `Left`, and `Right` to identify arrow keys.
  - Alternative names are `UpArrow`, `DownArrow`, `LeftArrow`, and `RightArrow` respectively.
- `Insert`, `Delete`, `Home`, `End`, `PgUp`, and `PgDown` to identify the cluster of special keys that sits atop the arrow keys.
- `F1` to `F15` to identify the function keys.
- `Numpad0` to `Numpad9`, `NumpadEquals`, `NumpadSlash`, `NumpadStar`, `NumpadMinus`, `NumpadPlus`, `NumpadPeriod`, and `NumpadEnter` to identify keys on the number pad.
- `LShift`, `RShift`, `LControl`, `RControl`, `LAlt`, `RAlt`, `LWin`, `RWin`, `Apps`, `Esc`, `Enter`, `Space`, `Backspace`, `Tab`, `NumLock`, `CapsLock`, and `ScrollLock` to identify the corresponding special keyboard keys.
- `Grave`, `Minus`, `Equals`, `LBracket`, `RBracket`, `Backslash`, `Semicolon`, `Apostrophe`, `Comma`, `Period`, and `Slash` to identify the corresponding symbol keys.
- Single letters (`A` to `Z`) or numbers (`0` to `9`) to identify the corresponding keys.

For example, the below configuration links the d-pad of an XInput controller to the arrow keys, the Start button to the Enter key, and the Back button to the Escape key.

```ini
[CustomMapper:KeyboardExample]

; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

DpadUp              = Keyboard(UpArrow)
DpadDown            = Keyboard(DownArrow)
DpadLeft            = Keyboard(LeftArrow)
DpadRight           = Keyboard(RightArrow)
ButtonStart         = Keyboard(Enter)
ButtonBack          = Keyboard(Esc)
```


#### Pov

A POV element mapper links an XInput controller element to the POV hat on a virtual controller. A single parameter is required specifying the associated POV hat direction: `Up`, `Down`, `Left`, or `Right`.


#### Null

A Null element mapper does nothing whatsoever and causes an XInput controller element to be ignored. No parameters are accepted.

This type of element mapper is primarily useful for removing element mappers from templates. For example, if the template is "StandardGamepad" and the goal is to cause the A button to be ignored, then the below will work.

```ini
[CustomMapper:NullExample]
Template            = StandardGamepad
ButtonA             = Null
```


#### Split

A Split element mapper requires two parameters, each of which is another element mapper. The first parameter is its "positive" element mapper and the second is its "negative" element mapper. If the assigned XInput controller element reports positive input (i.e. stick position is positive, button is pressed, or trigger is greater than the mid-point value) then the positive element mapper is asked to process the input, otherwise the negative element mapper is asked to do so. It is valid to specify "Null" as a parameter, with the outcome being that the corresponding input (positive or negative) is simply ignored.

The primary use case for a Split element mapper is to separate an XInput controller's analog stick axis into a positive part and a negative part. For example, the below configuration splits both axes of the left stick into a positive part and a negative part, triggering a different keyboard key in each case.

```ini
[CustomMapper:SplitExampleArrowKeys]

; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

; If the left stick's X axis is moved sufficiently to the right (positive), press the right arrow key.
; If the left stick's X axis is moved sufficiently to the left (negative), press the left arrow key.
; Of course, if the left stick's X axis is neutral, then neither key is pressed.
StickLeftX          = Split( Keyboard(Right), Keyboard(Left) )

; If the left stick's Y axis is moved sufficiently down (positive), press the down arrow key.
; If the left stick's Y axis is moved sufficiently up (negative), press the up arrow key.
; Of course, if the left stick's X axis is neutral, then neither key is pressed.
StickLeftY          = Split( Keyboard(Down), Keyboard(Up) )
```

Another possible use case is filtering. Suppose the goal is to map from the left stick's X axis on an XInput controller to virtual controller button 2, but only if the left stick is pressed in the positive direction (i.e. to the right). The configuration below would not adequately capture this goal because the button would be pressed irrespective of axis direction.

```ini
[CustomMapper:ButtonFromAxisExample]

; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

; If the left stick's X axis is pressed in either direction then button 2 is pressed.
StickLeftX          = Button(2)
```

A Split element mapper can be used to filter out all of the negative input so that the button is only considered pressed in the positive direction.

```ini
[CustomMapper:SplitButtonFromAxisExample]

; This example is not complete.
; It only defines element mappers for a small subset of controller elements.

; If the left stick's X axis is pressed to the right (positive) direction then button 2 is pressed.
StickLeftX          = Split( Button(2), Null )
```

Split element mappers behave somewhat differently depending if they are linked to an XInput axis, button, or trigger, as follows.
- If assigned to an **axis**...
  - If the axis reports a *neutral* or *positive* value then the positive element mapper is forwarded the analog value.
  - If the axis reports a *negative* value then the negative element mapper is forwarded the analog value.
- If assigned to a **button**...
  - If the button is *pressed*, the positive element mapper is sent a "button pressed" input.
  - If the button is *not pressed*, the negative element mapper is sent a "button pressed" input.
    - This behavior may be counter-intuitive.  Its supporting rationale is that sending a "button not pressed" input to an element mapper is functionally useless.
- If assigned to a **trigger**...
  - If the trigger is *pressed to at least at the midpoint position* then the positive element mapper is forwarded the trigger value.
  - If the trigger is either *not pressed* or *pressed below the midpoint position* then the negaive element mapper is forwarded the trigger value.


### Force Feedback Actuators

**Customizing the force feedback actuators is an advanced feature.**

A *force feedback actuator* is a physical device that produces a force feedback effect. For example, Xbox One controllers feature four such actuators:
 - Left motor, located in the body of the controller
 - Right motor, located in the body of the controller
 - Left impulse trigger, located near the LT trigger
 - Right impulse trigger, located near the RT trigger

Internally, the XInput API allows force feedback actuators to be used by specifying a rumble strength value, and these rumble strength values can vary with time to produce different vibration effects.

Force feedback in DirectInput is very different. While the concept does encompass vibration effects, it is much more general and also includes within its purview actual forces that a game controller device might apply to controller components. For example, a joystick might have an effect generator that is capable of exerting a force on the joystick such that it is pushed in a particular direction. Therefore, DirectInput exposes all force feedback effects as actual forces exerted along one or more axes. Following the joystick example, an application might request a force be exerted on the X axis, which would result in the joystick itself being pushed along the corresponding physical direction.

The idea of exerting a force along an axis does not make much sense in the context of vibration motors that accept a simple rumble strength value. DirectInput applications generally expect force feedback effects to be supported along both the X and Y axes, so Xidi provides that support. By default, Xidi computes the magnitude of the force vector along the X-Y plane and uses the resulting value as the rumble strength applied to both left and right motors. While this default behavior works in many situations, Xidi allows the mapping of axes to physical force feedback actuators to be customized.

In addition to controller components like DpadUp, TriggerLT, and ButtonA, there are a few specific force feedback actuators whose behaviors can be customized. The example below shows all supported force feedback actuators and the default values used for the built-in mappers.

```ini
[CustomMapper:DefaultForceFeedbackSettings]

; This example is not complete.
; It only defines force feedback actuators, nothing else.

; Left motor.
ForceFeedback.LeftMotor     = MagnitudeProjection(X, Y)

; Right motor.
ForceFeedback.LeftMotor     = MagnitudeProjection(X, Y)
```

Xidi internally supports all four actuators, but the documented XInput API only exposes the left and right motors. As a result, the impulse triggers are currently not available.

Force feedback actuator settings exist alongside the element mappers and are generally treated the same way when it comes to creating new custom mappers and using existing mappers as templates. However, there is one caveat: if a custom mapper does not use a template and also does not define any force feedback settings, then the default force feedback settings are applied, as shown in the preceding example. The rationale for this caveat is that it ensures users who want to build custom mappers from scratch but do not wish to concern themselves with force feedback settings are still able to use force feedback.

Various modes are supported for each force feedback actuator. These are described in the subsections that follow.


#### Disabled

A Disabled force feedback actuator does not produce any vibration effects whatsoever. This is primarily useful for removing a force feedback actuator from templates. For example, if the template is "StandardGamepad" and the goal is to cause the left motor to be turned off, then the below will work.

```ini
[CustomMapper:DisabledForceFeedbackExample]
Template                    = StandardGamepad
ForceFeedback.LeftMotor     = Disabled
```


#### MagnitudeProjection

A MagnitudeProjection force feedback actuator computes the magnitude of the force feedback effect along a two-axis plane and uses the result as the rumble strength. Two parameters are required, each identifying an axis. This is the default mode for the left and right motors, using the X and Y axes together to obtain the rumble strength, as shown in the default settings example.


#### SingleAxis

A SingleAxis force feedback actuator directly obtains its rumble strength from a single force feedback axis. A direction can optionally be specified so that either positive or negative axis values are filtered out. Using a bidirectional SingleAxis force feedback actuator causes the absolute value of the effect strength along that axis to be mapped to the rumble strength.

Parameters are the same as for the Axis and DigitalAxis element mappers. The first parameter identifies the axis of interest, and the second may optionally specify a direction. Refer to the example below for a demonstration of how to use SingleAxis force feedback actuators.

```ini
[CustomMapper:ForceFeedbackSingleAxisExample]

; Left motor rumble strength is the absolute value of the force effect's X component.
ForceFeedback.LeftMotor     = SingleAxis(X)

; Right motor rumble strength is 0 if the force effect's X component is positive, otherwise it is the absolute value of the force effect's X component.
ForceFeedback.RightMotor    = SingleAxis(X, -)
```


# Questions and Answers

#### Which specific controllers does Xidi support?

Any controller that supports Windows and XInput can be used with Xidi. This includes non-Microsoft controllers, wireless controllers, wired controllers, and so on. That being said, only Xbox 360 and Xbox One controllers have been specifically tested.

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

Yes, force feedback is implemented as of version 4.0.0.
