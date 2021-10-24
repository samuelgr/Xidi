# Xidi Developer Documentation

This documentation is for developers who wish to compile Xidi from source and potentially modify it. Topics include a compilation guide, a description of the organization of its source code, and an overview of the concepts that underlie its design and implementation.


## Building Xidi

The build system Xidi uses is based on Microsoft Visual Studio 2019 Community Edition. To build Xidi, simply open the supplied Visual Studio solution file and build from the graphical interface. One Visual C++ project exists for each form of Xidi along with an additional project for running unit tests. Each project supports building both 32-bit and 64-bit versions of Xidi. Debug and Release configurations will respectively produce debug (checked) and optimized (unchecked) versions of each library.


## Design and Implementation

This section provides a high-level description of how Xidi is designed and implemented. For those portions of this section that refer to XInput and DirectInput, it is assumed that the reader is familiar with these APIs and their relevant concepts, as described in Microsoft's documentation.

A zoomed-out view of Xidi's functionality boils down to a very short sequence of three steps:
 - *Read* the controller state of a real game controller using XInput
 - *Translate* said controller state from an XInput representation to an internal representation for a Xidi virtual controller device
 - *Expose* the state of Xidi virtual controller devices via DirectInput and WinMM

Of these three steps, *translate* is the most intricate and interesting. The *read* step simply involves invoking the relevant XInput API functions, and the *expose* step requires that Xidi implement DirectInput and WinMM interfaces as described in the relevant documentation.

Xidi's notion of virtual controller forms the heart of its implementation. A *virtual controller* is simply a layer of abstraction: it acts as an internal receptacle for translated controller data. Conversely, a *physical controller* is an actual XInput controller device, as exposed by the XInput API. The number of possible physical controllers is determined by the XInput API, but the number of virtual controller objects is not limited by the system or by Xidi. Multiple virtual controller objects can all be associated with the same physical controller device.

Xidi uses mappers to translate from raw XInput controller state to the state of a virtual controller device and subsequently exposes virtual controller state to applications. Mappers also determine the capabilities (number of axes, number of buttons, and so on) of the virtual controllers that Xidi exposes to applications.

Constants and data structures that are used to represent virtual controller data are defined in `ControllerTypes.h`, and the main top-level implementation of virtual controllers is in the **VirtualController** module. A virtual controller can contain up to 6 axes (X, Y, Z, X-rotation, Y-rotation, and Z-rotation), 16 buttons, and 1 POV hat. The mapper object in use determines which subset of these controller elements is actually present on the associated virtual controller and, accordingly, which elements of the virtual controller state data structure would ever contain valid data. Communication with physical controller devices via the XInput API is all contained in the **PhysicalController** module.

The sections that follow describe each step in Xidi's overall functionality in more detail.


### Reading XInput State

The **PhysicalController** module governs all communication with physical XInput controllers. A background thread runs periodically and polls for the state of all available XInput controllers. If a change in physical state is detected, any virtual controllers associated with the physical controller whose state changed are notified. Upon receiving such a notification, a virtual controller refreshes its state by taking into consideration the updated physical controller state information. Whenever an application requests the state of the virtual controller it is simply given the view that was created during the most recent state refresh operation.

Behind the scenes, the **PhysicalController** module spawns a background thread that periodically polls every possible XInput controller using `XInputGetState` and compares the results to the last known physical state of each controller. It also maintains one condition variable object per possible physical controller. If a change in state is detected for a physical controller, the state data structure for that controller is updated and the associated condition variable is signalled. On the receiving end of physical state data, each virtual controller object spawns a background thread that continually monitors the condition variable of its associated physical controller. On receiving a signal of physical state change, the virtual controller object reads the new physical state and uses it to update its own virtual state.


### Translating to Virtual Controller State

Translation from physical controller state to virtual controller state is governed entirely by mapper objects, one of which exists for each of the [documented mapper types](README.md). Internally each mapper object contains a set of *element mapper* objects, one for each XInput controller element. The subsections that follow describe how this process works.


#### Element Mappers

An *element mapper* reads the value associated with a single element of an XInput controller (i.e. A button, LT trigger, right-stick horizontal axis) and writes a value contribution to the data structure representing a virtual controller's state. Each element mapper is allowed to contribute to any number of elements of a virtual controller, and it is possible for multiple element mappers to contribute to the same element of a virtual controller. Element mappers are expected to be stateless with respect to previous or future contributions to virtual controller state.

"Contributing to a virtual controller element" means producing a value for the virtual controller element and then aggregating it with whatever value already exists for that element. This is important because multiple mappers might contribute to the same virtual controller element. For an element mapper that contributes to a virtual controller axis this typically means aggregation by summation: if an element mapper intends to produce a value of 1000 for its associated axis, rather than writing 1000 it should add 1000 to whatever value already exists for that axis.

Element mapper objects implement the `IElementMapper` interface defined in `ElementMapper.h`. `ContributeFromAnalogValue`, `ContributeFromButtonValue`, and `ContributeFromTriggerValue` all give the element mapper a chance to write its contribution to its associated virtual controller element. Where they differ is how the input value is obtained: from an analog stick (left or right, horizontal or vertical), from a digital button (A, B, X, Y, and so on), or from a trigger (LT or RT) respectively. In general element mappers are expected to be able to compute a contribution irrespective of the input source. The method `GetTargetElement` is used to identify the virtual controller element to which the element mapper writes its contribution.

Xidi provides four types of element mappers, each of which is described in the subsections that follow. All of these element mapper types are declared and documented in `ElementMapper.h`.


##### AxisMapper

This type of element mapper writes its output to a single axis on a virtual controller. The specific axis can be specified, and the mapper can be configured to contribute either to the entire axis or to just half of the axis (either positive or negative direction). Aggregation of multiple contributions to the same virtual controller axis occurs by summation.

If the input source is an analog or trigger value, AxisMapper passes the value right through to the target virtual controller axis. Half-axis mode primarily makes sense for triggers rather than full analog sticks. For example, attaching each trigger to the same axis but opposite polarity results in both triggers sharing the axis.

If the input source is a button, then the output is either an extreme axis position or neutral. In whole-axis mode, the output is extreme positive if the button is pressed and extreme negative otherwise. In half-axis mode, the output is neutral (i.e. center position) if the button is not pressed and extreme in the configured direction if it is pressed.


##### DigitalAxisMapper

Largely equivalent to AxisMapper but forces all output to be digital. There is no difference for input from digital buttons. For analog and trigger inputs the output contribution is either neutral position or one of the extremes.


##### ButtonMapper

This type of element mapper writes its output to a single digital button on a virtual controller. The specific button can be specified. Aggregation of multiple contributions to the same virtual controller button occurs by means of logical or.

If the input source is an analog or trigger value, the displacement from neutral is compared with a threshold. If the displacement is greater than the threshold the button is pressed, otherwise it is not pressed. In this context, "displacement from neutral" considers centered to be neutral for analog sticks and completely unpressed to be neutral for triggers.

If the input source is a button value, the output button state is the same as the input.


##### KeyboardMapper

Behavior is very similar to ButtonMapper in terms of the logic. However, instead of contributing to a virtual controller button press, this type of mapper simulates a key press on the system keyboard. Keys are identified by DirectInput scan code, which are listed as `DIK_*` constants in the file `dinput.h`.


##### PovMapper

This type of element mapper writes its output to the virtual controller's POV hat. One or two POV directions can be specified: the first is the *positive* direction and the optional second is the *negative* direction. Xidi internally treats each POV direction as its own individual digital button, so all contributions to POV directions are aggregated using logical or.

If the input source is an analog stick value, then the input value is compared with a threshold. If the input value is positive and greater than the threshold then the positive direction is pressed. If the input value is negative, its absolute value exceeds the threshold, and a negative direction is configured, then the negative direction is pressed. If none of these conditions hold then the PovMapper does not cause any POV directions to be pressed.

If the input source is a trigger value, then the input value is compared with a threshold. If the input value is greater than the threshold then the positive direction is pressed. If not, and a negative direction is configured, then the negative direction is pressed. If none of these conditions hold then the PovMapper does not cause any POV directions to be pressed.

If the input source is a button value, then the same logic applies as with a trigger value. However, instead of comparing with a threshold, the decision is based on whether or not the input button is pressed.


##### SplitMapper

This is a multi-element mapper which contains within it two underlying element mappers. Which underlying mapper is used for any given contribution is determined by whether or not the incoming input source value is considered positive or negative. The implementation allows for either or both of the underlying mappers to be omitted.

If the input source is an analog stick value, then the positive mapper is used if the raw value is non-negative, and the negative mapper is used otherwise. A good use case for this scenario is to split an XInput axis into a positive half and a negative half. For example, suppose a `SplitMapper` is used on the left stick X axis such that the negative mapper is a `ButtonMapper` assigned to button 1 and the positive mapper is a `ButtonMapper` assigned to button 2. Whenever the user pushes the left stick far enough to the left virtual button 1 is pressed, and whenever the user pushes the left stick far enough to the right virtual button 2 is pressed.

If the input source is a trigger value, then the positive mapper is used if the raw value is at least equal to the mid-point value for triggers, and the negative mapper is used otherwise.

If the input source is a button value, then the positive mapper is used if the raw value indicates that the button is pressed, and the negative value is used otherwise.


#### Top-Level Mappers

Individual element mappers define how one input value is transformed to a contribution to one virtual controller element. Top-level mapper objects, instances of the `Mapper` class declared in `Mapper.h`, aggregate several element mappers together to produce one overall mapper for an entire virtual controller. At most one element mapper object exists for each XInput controller element. Setting an element mapper to `nullptr` causes the top-level mapper to ignore input from that XInput controller element.

When a mapper object is constructed, it iterates through all of its associated element mappers and queries them by invoking `GetTargetElement`. This information allows the top-level mapper to determine the capabilities of the virtual controller. For example, if none of the element mappers identify Z-rotation axis as the target element, it follows that the virtual controller does not have a Z-rotation axis. The logic for buttons is slightly different: the number of buttons is determined by the highest button identified by any element mappers, even if that means skipping some. For example, if element mappers identify buttons 1, 3, and 6, the virtual controller has 6 buttons even though 2, 4, and 5 are never pressed.

A mapper object's most frequent request is to translate from XInput controller state to virtual controller state.  During the processing of such a request, the mapper object iterates through all of its associated element mappers and invokes the correct contribution method: `ContributeFromAnalogValue` for element mappers associated with analog sticks, `ContributeFromButtonValue` for element mappers associated with digital buttons, and `ContributeFromTriggerValue` for element mappers associated with the left and right triggers.

Mapper objects are instantiated as constants in the file `MapperDefinitions.cpp`. This is where all of the documented mapper types can be found. To create a new mapper type, append an entry to the array contained in that file.


### Exposing Virtual Controllers to Applications

The modules **VirtualDirectInputDevice**, **WrapperIDirectInput**, and **WrapperJoyWinMM** expose virtual controllers to applications using DirectInput and WinMM. Internally, objects of the class `VirtualDirectInputDevice` use instances of the `DataFormat` class to manipulate data packets in the format specified by the application.

Implementations of all of these modules are intended to be as straightforward as possible. The goal is to implement the relevant APIs per Microsoft documentation and observed behavior.


## Source Code Organization

Source code documentation is available and can be built using Doxygen. This section provides an overview of how the source code is organized, with reference to the concepts and high-level design details discussed previously.

**ApiDirectInput** and **ApiGUID** provide helpers for interacting with the DirectInput API. In the former case, the goal is to ensure definitions of constant-valued GUIDs are embedded into Xidi because Xidi cannot load them from the system-supplied DirectInput library. In the latter case, the goal is to provide methods of hashing and comparing GUID values so they may be used in STL containers.

**Configuration** provides the functionality needed to parse and apply configuration files. Supported values and section names are defined statically using STL `unordered_map` containers. Each value is associated with a function to be invoked when the particular configuration value is applied from a configuration file. These functions return success or failure depending on the semantic validity of the value that is specified. The main control flow for the process of reading a configuration file is contained in the method `ParseAndApplyConfigurationFile`.

**ControllerIdentification** provides helpers for identifying and enumerating XInput and non-XInput controllers. This class is used primarily during the controller enumeration process. Xidi statically defines its own GUIDs for identifying Xidi virtual controllers in a manner compatible with the DirectInput API specification. The `EnumerateXInputControllers` methods perform a DirectInput-style enumeration of the Xidi virtual controllers to the application and return whatever status code the application's callback function supplies. Other methods are available for manipulating GUIDs that represent Xidi virtual controllers.

**DataFormat** is a helper class for manipulating data packets in the format specified by a DirectInput application. A key concept of DirectInput is that applications are allowed to tell the system how they expect controller data to be formatted in memory. Accordingly, the functionality offered by objects of this class is related to translating between Xidi's internal data format for virtual controllers and an application's requested DirectInput data format. Instances of this class are created via a factory method that parses an application's data format specification, which an application provides when it invokes the `IDirectInputDevice::SetDataFormat` method.

**DirectInputClassFactory** implements a COM class factory interface. The HookModule form of Xidi invokes `DllGetClassObject`, a standard COM function, which in turn makes use of the functionality contained in this module to provide a standard COM interface for constructing top-level DirectInput interface objects.

**ElementMapper** is where the `IElementMapper` interface is defined and all of the element mapper types are implemented.

**ExportApiDirectInput** and **ExportApiWinMM** implement the external interfaces to Xidi, mimicking the interfaces exposed by the system-supplied versions of the DirectInput and WinMM libraries. Applications that load Xidi will invoke these functions directly. In many cases they simply pass through to the imported functions of the same name, but when needed they perform additional functionality, calling into other parts of Xidi.

**Globals** holds miscellaneous global values and provides methods for accessing them. Examples include the specified import library paths from the configuration file and the internal system-defined instance handle used to refer specifically to the Xidi library.

**ImportApiDirectInput** and **ImportApiWinMM** hold the addresses of all functions imported from either the system-supplied or user-overridden DirectInput and WinMM libraries. Other classes may call methods defined by these classes to invoke imported functions. The import tables are lazily initialized on first access. If an imported function is called that was missing from the originally-loaded library, these classes intentionally cause the program to crash by invoking a function using a `NULL` pointer. If logging is enabled, a message is logged indicating the problematic function.

**Keyboard** tracks virtual keyboard state as reported by any `KeyboardMapper` objects that may exist. It maintains state information for each key on the virtual keyboard and periodically submits keyboard events to the system using the `SendInput` Windows API function.

**Log** implements all functionality related to logging. It accepts configuration settings regarding the minimum required severity, determines which log messages to output and which to ignore, creates the log file when needed, flushes it on program termination, and handles all output to it. Various ways of generating log messages are also implemented, including specifying a string directly or loading a string from a resource embedded in the binary. String generation is separated from file output because in the future it may be desirable to support logging to somewhere other than a file, such as to a graphical interface via inter-process communciation.

**Mapper** contains the declaration and implementation of top-level mapper objects.

**MapperBuilder** implements all custom mapper building functionality. A single `MapperBuilder` object holds custom mapper blueprint objects, each of which contains a description of the desired contents of a custom mapper. Once all custom mapper candidates are fully parsed from the configuration file, this object attempts to resolve all template dependencies and construct mapper objects.

**MapperDefinitions** contains instantiations of the built-in mappers.

**MapperParser** implements all string-parsing functionality for identifying XInput controller elements and constructing element mappers based on strings contained within a configuration file.

**PhysicalController** manages all communication with the underlying XInput API. It periodically polls devices for changes to physical state and supports notifying other modules whenever a physical state change is detected.

**StateChangeEventBuffer** is a helper for virtual controller objects that allows them to support event buffering, which is in turn used to expose DirectInput buffered events to applications.

**VirtualController** is the top-level virtual controller implementation. It combines all of the individual units of functionality needed to present a cohesive controller interface, including mapping, event buffering, and even some configuration properties. Some of the functionality is guided by what DirectInput expects, although none of the implementation is DirectInput-specific.

**VirtualDirectInputDevice** is a DirectInput interface for exposing Xidi virtual controllers to applications. This class implements IDirectInputDevice (or IDirectInputDevice8, depending on the compiled form of Xidi) and contains a Xidi virtual controller device instance with which it communicates internally. Functionality related to application-defined data format is delegated to the DataFormat helper class.

**WrapperIDirectInput** and **WrapperJoyWinMM** act as interception classes for all calls to DirectInput (via IDirectInput or IDirectInput8) and WinMM joystick (via direct function calls) APIs. In the former case, an instance of WrapperIDirectInput is returned when the application calls one of the DirectInput object creation methods exported by the Xidi DLL, and in the latter case, the joystick family of WinMM calls invokes functions supplied by WrapperJoyWinMM.

**XidiConfigReader** specializes the generic functionality offered by the Configuration subsystem and defines the allowed format of a Xidi configuration file.
