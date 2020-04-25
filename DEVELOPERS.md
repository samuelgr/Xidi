# Xidi Developer Documentation

This documentation is for developers who wish to compile Xidi from source and potentially modify it. Topics include a compilation guide, a description of the organization of its source code, and an overview of the concepts that underlie its implementation.


## Building Xidi

The build system Xidi uses is based on Microsoft Visual Studio 2019 Community Edition. To build Xidi, simply open the supplied Visual Studio solution file and build from the graphical interface. One Visual C++ project exists for each version of Xidi (`dinput.dll`, `dinput8.dll`, and `winmm.dll`), and each supports building both 32-bit and 64-bit versions of Xidi.

Debug and Release configurations will respectively produce debug (checked) and optimized (unchecked) versions of each library. A third configuration, TestApp, produces an executable that is designed as a standalone test suite for Xidi's functionality. This application runs through a series of API tests designed to ensure compliance with documented API specifications for WinMM and DirectInput. It includes an interactive controller test mode at the end, in which buttons on an attached XInput controller can be pressed and the results reflected on screen.


## Source Code

Source code documentation is available and can be built using Doxygen.
The remainder of this section provides an overview of how the source code is organized and, at a high level, how Xidi is implemented.

**ApiDirectInput** and **ApiGUID** provide helpers for interacting with the DirectInput API. In the former case, the goal is to ensure definitions of constant-valued GUIDs are embedded into Xidi because Xidi cannot load them from the system-supplied DirectInput library. In the latter case, the goal is to provide methods of hashing and comparing GUID values so they may be used in STL containers.

**Configuration** provides the functionality needed to parse and apply configuration files. Supported values and section names are defined statically using STL `unordered_map` containers. Each value is associated with a function to be invoked when the particular configuration value is applied from a configuration file. These functions return success or failure depending on the semantic validity of the value that is specified. The main control flow for the process of reading a configuration file is contained in the method `ParseAndApplyConfigurationFile()`.

**ControllerIdentification** provides helpers for identifying and enumerating XInput and non-XInput controllers. This class is used primarily during the controller enumeration process. Xidi statically defines its own GUIDs for identifying Xidi virtual controllers in a manner compatible with the DirectInput API specification. The `EnumerateXInputControllers()` methods perform a DirectInput-style enumeration of the Xidi virtual controllers to the application and return whatever status code the application's callback function supplies. Other methods are available for manipulating GUIDs that represent Xidi virtual controllers.

**ExportApiDirectInput** and **ExportApiWinMM** implement the external interfaces to Xidi, mimicking the interfaces exposed by the system-supplied versions of the DirectInput and WinMM libraries. Applications that load Xidi will invoke these functions directly. In many cases they simply pass through to the imported functions of the same name, but when needed they perform additional functionality, calling into other parts of Xidi.

**Globals** holds miscellaneous global values and provides methods for accessing them. Examples include the specified import library paths from the configuration file and the internal system-defined instance handle used to refer specifically to the Xidi library.

**ImportApiDirectInput** and **ImportApiWinMM** hold the addresses of all functions imported from either the system-supplied or user-overridden DirectInput and WinMM libraries. Other classes may call methods defined by these classes to invoke imported functions. The import tables are lazily initialized on first access. If an imported function is called that was missing from the originally-loaded library, these classes intentionally cause the program to crash by invoking a function using a `NULL` pointer. If logging is enabled, a message is logged indicating the problematic function.

**Log** implements all functionality related to logging. It accepts configuration settings regarding the minimum required severity, determines which log messages to output and which to ignore, creates the log file when needed, flushes it on program termination, and handles all output to it. Various ways of generating log messages are also implemented, including specifying a string directly or loading a string from a resource embedded in the binary. String generation is separated from file output because in the future it may be desirable to support logging to somewhere other than a file, such as to a graphical interface via inter-process communciation.

**MapperFactory** encapsulates all functionality related to the construction of objects that map DirectInput controller objects to XInput controller elements. Based on the configured type of mapper (either the default or overridden in a configuration file), the `CreateMapper()` method constructs and returns a mapper object.

**TestAppDirectInput** and **TestAppWinMM** implement the test applications for each interface respectively. Simple checks are performed for API compliance and consistency, and an interactive mode at the end allows controllers and mappers to be tested directly by a user. These applications are console-based and quite bare-bones.

**VirtualDirectInputDevice** implements the IDirectInputDevice and IDirectInputDevice8 interfaces presented to applications and used to provide Xidi virtual controller devices. Each such object communicates with both a mapper object and an XInput controller object which together provide the needed functionality.

**WrapperIDirectInput** and **WrapperJoyWinMM** act as interception classes for all calls to DirectInput (via IDirectInput or IDirectInput8) and WinMM joystick (via direct function calls) APIs. In the former case, an instance of WrapperIDirectInput is returned when the application calls one of the DirectInput object creation methods exported by the Xidi DLL, and in the latter case, the joystick family of WinMM calls invokes functions supplied by WrapperJoyWinMM.

**XInputController** implements communication with XInput-based controllers, translating between DirectInput and XInput as needed. Many DirectInput-specific operations are emulated, and a DirectInput-like interface is presented. Both WrapperIDirectInput and WrapperJoyWinMM communicate with XInput-based controllers thorugh this class.

**Mapper::Base** provides the key mapping functionality for translating between DirectInput controller objects and XInput controller elements. Its subclasses specify the exact component mappings, but the base class implements all the logic required to perform the actual mapping operations. These operations include setting and honoring properties (range, deadzone, and so on) on axes, enumerating controller objects for applications, and parsing the format specification an application supplies for retrieving controller state. This last operation, implemented in `SetApplicationDataFormat()`, is the most complex; Xidi supports both Microsoft's built-in format specifications as well as custom format specifications.

**Mapper::StandardGamepad**, **Mapper::ExtendedGamepad**, **Mapper::XInputNative**, and **Mapper::XInputSharedTriggers** are subclasses of Mapper::Base and specify the precise mappings of DirectInput controller objects to XInput controller elements. Creating a new mapping is as simple as creating a new subclass of Mapper::Base and overriding the required virtual methods.
