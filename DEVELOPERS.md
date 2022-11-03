# Xidi Developer Documentation

This documentation is for developers who wish to compile Xidi from source and potentially modify it. Topics include a compilation guide, a description of the organization of its source code, and an overview of the concepts that underlie its design and implementation.


## Building Xidi

The build system Xidi uses is based on Microsoft Visual Studio 2022. To build Xidi, simply open the supplied Visual Studio solution file and build from the graphical interface. One Visual C++ project exists for each form of Xidi along with an additional project for running unit tests. Each project supports building both 32-bit and 64-bit versions of Xidi. Debug and Release configurations will respectively produce debug (checked) and optimized (unchecked) versions of each library.


## Design and Implementation

This section provides a high-level description of how Xidi is designed and implemented. For those portions of this section that refer to XInput and DirectInput, it is assumed that the reader is familiar with these APIs and their relevant concepts, as described in Microsoft's documentation.

A zoomed-out view of Xidi's input functionality boils down to a very short sequence of three steps:
 - *Read* the controller state of a real game controller using XInput
 - *Translate* said controller state from an XInput representation to an internal representation for a Xidi virtual controller device
 - *Expose* the state of Xidi virtual controller devices via DirectInput and WinMM

Of these three steps, *translate* is the most intricate and interesting. The *read* step simply involves invoking the relevant XInput API functions, and the *expose* step requires that Xidi implement DirectInput and WinMM interfaces as described in the relevant documentation.

Xidi also supports controller vibration, also known as *force feedback* in DirectInput parlance. The steps are basically the opposite as above, as Xidi must translate between the DirectInput application's view of the controller and the real physical XInput controller.

Xidi's notion of virtual controller forms the heart of its implementation. A *virtual controller* is simply a layer of abstraction: it acts as an internal receptacle for translated controller data. Conversely, a *physical controller* is an actual XInput controller device, as exposed by the XInput API. The number of possible physical controllers is determined by the XInput API, but the number of virtual controller objects is not limited by the system or by Xidi. Multiple virtual controller objects can all be associated with the same physical controller device.

Xidi uses mappers to translate between XInput controller data and virtual controller data, and subsequently exposes virtual controller data to applications. Mappers also determine the capabilities (number of axes, number of buttons, whether or not force feedback is supported, and so on) of the virtual controllers that Xidi exposes to applications.

Constants and data structures that are used to represent virtual controller data are defined in `ControllerTypes.h`, with certain pieces specific to force feedback being in `ForceFeedbackTypes.h`. The main top-level implementation of virtual controllers is in the VirtualController module. A virtual controller can contain up to 6 axes (X, Y, Z, X-rotation, Y-rotation, and Z-rotation), 16 buttons, and 1 POV hat. The mapper object in use determines which subset of these controller elements is actually present on the associated virtual controller and, accordingly, which elements of the virtual controller state data structure would ever contain valid data. Communication with physical controller devices via the XInput API is all contained in the PhysicalController module.

The sections that follow describe each step in Xidi's overall functionality in more detail.


### Reading XInput State

PhysicalController governs all communication with physical XInput controllers. A background thread runs periodically and polls for the state of all available XInput controllers. If a change in physical state is detected, any virtual controllers associated with the physical controller whose state changed are notified. Upon receiving such a notification, a virtual controller refreshes its state by taking into consideration the updated physical controller state information. Whenever an application requests the state of the virtual controller it is simply given the view that was created during the most recent state refresh operation.

Behind the scenes, the PhysicalController module spawns a background thread that periodically polls every possible XInput controller using `XInputGetState` and compares the results to the last known physical state of each controller. It also maintains one condition variable object per possible physical controller. If a change in state is detected for a physical controller, the state data structure for that controller is updated and the associated condition variable is signalled. On the receiving end of physical state data, each virtual controller object spawns a background thread that continually monitors the condition variable of its associated physical controller. On receiving a signal of physical state change, the virtual controller object reads the new physical state and uses it to update its own virtual state.


### Translating to Virtual Controller State

Translation from physical controller state to virtual controller state is governed entirely by mapper objects. One such instance exists for each of the [built-in mapper types](https://github.com/samuelgr/Xidi#built-in-mappers), and more instances can be created at run-time as [custom mappers](https://github.com/samuelgr/Xidi#custom-mappers). Internally each mapper object contains a set of *element mapper* objects, one for each XInput controller element. The subsections that follow describe how this process works.


#### Element Mappers

An *element mapper* reads the value associated with a single element of an XInput controller (i.e. A button, LT trigger, right-stick horizontal axis) and writes a value contribution to the data structure representing a virtual controller's state. Each element mapper is allowed to contribute to any number of elements of a virtual controller, and it is possible for multiple element mappers to contribute to the same element of a virtual controller. Certain types of element mappers may also have side effects which extend beyond simply updating virtual controller state.

Element mappers are expected to be stateless with respect to previous or future contributions to virtual controller state and side effects. However, each time it is asked to make a contribution, it is provided with an opaque "source identifier" which is an integer that uniquely identifies the source of the controller input that is triggering the contribution. There is no semantic meaning or guarantee as to the specific value or relationship between different source identifiers other than that they will be equal if they represent the same physical controller element on the same physical controller. For example, the same source identifier will be supplied to element mappers for all contributions from the "A" button on the controller associated with player 2, but any contributions from other controller elements or even the "A" button on a different player's controller will lead to a different source identifier. Source identifiers are useful for certain element mappers that produce side effects as a way of opaquely keeping track of contributions from different physical controller elements.

"Contributing to a virtual controller element" means producing a value for the virtual controller element and then aggregating it with whatever value already exists for that element. This is important because multiple mappers might contribute to the same virtual controller element. For an element mapper that contributes to a virtual controller axis this typically means aggregation by summation: if an element mapper intends to produce a value of 1000 for its associated axis, rather than writing 1000 it should add 1000 to whatever value already exists for that axis.

Element mapper objects implement the `IElementMapper` interface defined in `ElementMapper.h`. `ContributeFromAnalogValue`, `ContributeFromButtonValue`, and `ContributeFromTriggerValue` all give the element mapper a chance to write its contribution to its associated virtual controller element. Where they differ is how the input value is obtained: from an analog stick (left or right, horizontal or vertical), from a digital button (A, B, X, Y, d-pad direction, and so on), or from a trigger (LT or RT) respectively. By implementing all three of these methods element mappers are able to compute a contribution irrespective of the input source. The methods `GetTargetElementCount` and `GetTargetElementAt` are used to identify the virtual controller elements to which the element mapper writes its contribution. An additional optional method, `ContributeNeutral`, exists to allow those element mappers with side effects to undo those side effects in the absence of input.

Xidi provides several types of element mappers, each of which is described in the subsections that follow. All of these element mapper types are declared and documented in `ElementMapper.h`.


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


##### CompoundMapper

This type of element mapper contains an array of underlying element mappers and forwards any input received to all of them. Its methods do nothing special beyond iterating over the array and invoking the corresponding methods on all of its underlying element mappers.


##### InvertMapper

This type of element mapper contains a single underlying element mapper to which it forwards any input received after applying a transformation in the form of an inversion. Its methods implement the transformation and then invoke the corresponding methods on the underlying element mapper.


##### KeyboardMapper

Behavior is very similar to ButtonMapper in terms of the logic. However, instead of contributing to a virtual controller button press, this type of mapper simulates a key press on the system keyboard. Keys are identified by DirectInput scan code, which are listed as `DIK_*` constants in the file `dinput.h`.

This type of mapper is considered to have a side effect because, unlike other types, it does not not contribute directly to virtual controller state but rather to the keyboard state. As a result it implements `ContributeNeutral` so that associated keyboard buttons can be released in the absence of input from the controller.


##### MouseButtonMapper

Behavior and implementation is extremely similar to KeyboardMapper. However, instead of a virtual keyboard key, this type of element mapper simulates a mouse button press on the system mouse.


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

Because element mappers of this type have multiple underlying element mappers but only forward input to one of them (the "active" element mapper), whichever underlying element mapper is not asked for a contribution (the "inactive" element mapper) may have side effects that need to be undone. As a result, whenever the active element mapper is asked for a proper contribution the inactive element mapper is asked for a neutral contribution via `ContributeNeutral`.


#### Top-Level Mappers

Individual element mappers define how one input value is transformed to a contribution to one virtual controller element. Top-level mapper objects, instances of the `Mapper` class declared in `Mapper.h`, aggregate several element mappers together to produce one overall mapper for an entire virtual controller. At most one element mapper object exists for each XInput controller element. Setting an element mapper to `nullptr` causes the top-level mapper to ignore input from that XInput controller element.

When a mapper object is constructed, it iterates through all of its associated element mappers and queries them by invoking `GetTargetElement`. This information allows the top-level mapper to determine the capabilities of the virtual controller. For example, if none of the element mappers identify Z-rotation axis as the target element, it follows that the virtual controller does not have a Z-rotation axis. The logic for buttons is slightly different: the number of buttons is determined by the highest button identified by any element mappers, even if that means skipping some. For example, if element mappers identify buttons 1, 3, and 6, the virtual controller has 6 buttons even though 2, 4, and 5 are never pressed.

A mapper object's most frequent request is to translate from XInput controller state to virtual controller state.  During the processing of such a request, the mapper object iterates through all of its associated element mappers and invokes the correct contribution method: `ContributeFromAnalogValue` for element mappers associated with analog sticks, `ContributeFromButtonValue` for element mappers associated with digital buttons, and `ContributeFromTriggerValue` for element mappers associated with the left and right triggers.

Built-in mapper objects are instantiated as constants in the file `MapperDefinitions.cpp`. This is where all of the documented mapper types can be found. To create a new built-in mapper type, append an entry to the array contained in that file. Otherwise, new types of mappers can be created at run-time as custom mappers parsed from configuration files, the functionality for which is spread across `MapperBuilder.cpp` and `MapperParser.cpp`.


### Exposing Virtual Controllers to Applications

VirtualDirectInputDevice, WrapperIDirectInput, and WrapperJoyWinMM expose virtual controllers to applications using DirectInput and WinMM. Internally, objects of the class `VirtualDirectInputDevice` use instances of the `DataFormat` class to manipulate data packets in the format specified by the application.
Implementations of all of these modules are intended to be as straightforward as possible. The goal is to implement the relevant APIs per Microsoft documentation and observed behavior.


### Force Feedback

VirtualDirectInputEffect is Xidi's implementation of the IDirectInputEffect interface. Each such object is linked to an instance of VirtualDirectInputDevice to enable a proper implementation of all the interface methods.

Typically a DirectInput device vendor supplies a force feedback driver that exposes a physical device's hardware force feedback effect generator to applications via DirectInput. Whereas the XInput API is relatively simple and works by means of directly specifying the rumble strength for each individual motor, DirectInput allows much more semantically rich effect intentions to be expressed, leaving it to the combination of hardware and driver to deliver the desired output.

Xidi's emulation of force feedback effect generator hardware is encapsulated in the ForceFeedback namespace. The Effect hierarchy implements the effects themselves, exposing a convenient interface in which magnitude can be computed as a function of time. Mappers can then translate the output magnitude vectors, which are expressed using coordinates along DirectInput axes (X axis, Y axis, and so on), into physical actuator values with components that map directly to physical force feedback actuators (left motor, right motor, and so on). The PhysicalController module invokes the `XInputSetState` function periodically to set these physical force actuator values.

DirectInput requires that an applicaton acquire a controller in exclusive mode before playing force feedback effects. Xidi requires this too, and the way this works internally is that the PhysicalController tracks the virtual controller object that has acquired it. This way, the PhysicalController module is able to query the virtual controller, enlisting its help in mapping from a DirectInput magnitude vector to a physical force feedback actuator vector that can then be applied to the motors on the device. Mappers are, after all, a virtual controller concept rather than a physical controller concept; mapper objects are owned by virtual controllers, not by physical controllers.

Force feedback effect objects are intended to be uniquely identifiable but also exist in multiple locations. The rationale is that this enables Xidi to separate the parameters of an effect that exist in software - available via IDirectInputEffect - from the parameters that have been "downloaded" to the force feedback device. A new unique identifier is established whenever a new object is created, and when objects are copied they retain the same identifier. "Downloading an effect" is implemented by making a copy of the effect and placing it into a buffer owned by the force feedback device or, in the event of modifying an existing effect's parameters, invoking the `SyncParametersFrom` method on the device's version of the effect to update the parameters on the device and passing the software view as a source from which to obtain the new parameter values. Since the unique identifiers are identical Xidi knows that the two effect objects are intended to refer to the same effect and that the parameter transfer can be made in a type-safe way. By the same token, "unloading an effect" from the device is as simple as erasing the device's copy of the effect from the device buffer.


## Source Code Organization

Source code documentation is available and can be built using Doxygen. This section provides an overview of how the source code is organized, with reference to the concepts and high-level design details discussed previously.

**ApiDirectInput** and **ApiGUID** provide helpers for interacting with the DirectInput API. In the former case, the goal is to ensure definitions of constant-valued GUIDs are embedded into Xidi because Xidi cannot load them from the system-supplied DirectInput library. In the latter case, the goal is to provide methods of hashing and comparing GUID values so they may be used in STL containers.

**ApiXidi** implements an internal API currently used for communication between the HookModule and WinMM forms of Xidi to ensure proper functioning of the latter when system-supplied WinMM joystick functions are hooked.

**Configuration** provides the functionality needed to parse and apply configuration files. Supported values and section names are defined statically using STL `unordered_map` containers. Each value is associated with a function to be invoked when the particular configuration value is applied from a configuration file. These functions return success or failure depending on the semantic validity of the value that is specified. The main control flow for the process of reading a configuration file is contained in the method `ParseAndApplyConfigurationFile`.

**ControllerIdentification** provides helpers for identifying and enumerating XInput and non-XInput controllers. This class is used primarily during the controller enumeration process. Xidi statically defines its own GUIDs for identifying Xidi virtual controllers in a manner compatible with the DirectInput API specification. The `EnumerateXInputControllers` methods perform a DirectInput-style enumeration of the Xidi virtual controllers to the application and return whatever status code the application's callback function supplies. Other methods are available for manipulating GUIDs that represent Xidi virtual controllers.

**DataFormat** is a helper class for manipulating data packets in the format specified by a DirectInput application. A key concept of DirectInput is that applications are allowed to tell the system how they expect controller data to be formatted in memory. Accordingly, the functionality offered by objects of this class is related to translating between Xidi's internal data format for virtual controllers and an application's requested DirectInput data format. Instances of this class are created via a factory method that parses an application's data format specification, which an application provides when it invokes the `IDirectInputDevice::SetDataFormat` method.

**DirectInputClassFactory** implements a COM class factory interface. The HookModule form of Xidi invokes `DllGetClassObject`, a standard COM function, which in turn makes use of the functionality contained in this module to provide a standard COM interface for constructing top-level DirectInput interface objects.

**ElementMapper** is where the `IElementMapper` interface is defined and all of the element mapper types are implemented.

**ExportApiDirectInput** and **ExportApiWinMM** implement the external interfaces to Xidi, mimicking the interfaces exposed by the system-supplied versions of the DirectInput and WinMM libraries. Applications that load Xidi will invoke these functions directly. In many cases they simply pass through to the imported functions of the same name, but when needed they perform additional functionality, calling into other parts of Xidi.

**ForceFeedbackDevice** contains the top-level object Xidi uses to emulate force feedback devices. Methods encompass many of the sorts of operations that are typically performed on such devices via DirectInput. One instance of a force feedback device object is associated with each physical controller.

**ForceFeedbackEffect** contains the entire class hierarchy for implementing individual force feedback effects and determining their magnitudes as a function of time. For example, implementing a sine wave effect requires computing the trigonometric sine, which is implemented by one of the classes in this module. Parameter updates, retrieval, and validation are also handled by the various classes in this class hierarchy.

**ForceFeedbackParameters** defines data structures and objects that pertain to the parameters associated with force feedback effects. The largest implementation in this module is the direction vector, which is what Xidi uses to project a force of given magnitude along whatever axes, and in whatever direction, the application requested in the force feedback effect parameters.

**Globals** holds miscellaneous global values and provides methods for accessing them. Examples include the specified import library paths from the configuration file and the internal system-defined instance handle used to refer specifically to the Xidi library.

**ImportApiDirectInput** and **ImportApiWinMM** hold the addresses of all functions imported from either the system-supplied or user-overridden DirectInput and WinMM libraries. Other classes may call methods defined by these classes to invoke imported functions. The import tables are lazily initialized on first access. If an imported function is called that was missing from the originally-loaded library, these classes intentionally cause the program to crash by invoking a function using a `NULL` pointer. If logging is enabled, a message is logged indicating the problematic function.

**Keyboard** tracks virtual keyboard state as reported by any `KeyboardMapper` objects that may exist. It maintains state information for each key on the virtual keyboard and periodically submits keyboard events to the system using the `SendInput` Windows API function.

**Log** implements all functionality related to logging. It accepts configuration settings regarding the minimum required severity, determines which log messages to output and which to ignore, creates the log file when needed, flushes it on program termination, and handles all output to it. Various ways of generating log messages are also implemented, including specifying a string directly or loading a string from a resource embedded in the binary. String generation is separated from file output because in the future it may be desirable to support logging to somewhere other than a file, such as to a graphical interface via inter-process communciation.

**Mapper** contains the declaration and implementation of top-level mapper objects.

**MapperBuilder** implements all custom mapper building functionality. A single `MapperBuilder` object holds custom mapper blueprint objects, each of which contains a description of the desired contents of a custom mapper. Once all custom mapper candidates are fully parsed from the configuration file, this object attempts to resolve all template dependencies and construct mapper objects.

**MapperDefinitions** contains instantiations of the built-in mappers.

**MapperParser** implements all string-parsing functionality for identifying XInput controller elements, identifying force feedback actuators, and constructing both of these types of objects based on strings contained within a configuration file.

**Mouse** tracks virtual mouse state as reported by any `MouseButtonMapper` objects that may exist. It maintains state information for each possible mouse button and periodically submits mouse events to the system using the `SendInput` Windows API function.

**PhysicalController** manages all communication with the underlying XInput API. It periodically polls devices for changes to physical state and supports notifying other modules whenever a physical state change is detected.

**StateChangeEventBuffer** is a helper for virtual controller objects that allows them to support event buffering, which is in turn used to expose DirectInput buffered events to applications.

**VirtualController** is the top-level virtual controller implementation. It combines all of the individual units of functionality needed to present a cohesive controller interface, including mapping, event buffering, and even some configuration properties. Some of the functionality is guided by what DirectInput expects, although none of the implementation is DirectInput-specific.

**VirtualDirectInputDevice** is a DirectInput interface for exposing Xidi virtual controllers to applications. This class implements IDirectInputDevice (or IDirectInputDevice8, depending on the compiled form of Xidi) and contains a Xidi virtual controller device instance with which it communicates internally. Functionality related to application-defined data format is delegated to the DataFormat helper class.

**VirtualDirectInputEffect** is a DirectInput interface for exposing force feedback effect objects to applications. This module contains an entire class hierarchy of different effect types, all of which implement the IDirectInputEffect interface.

**WrapperIDirectInput** and **WrapperJoyWinMM** act as interception classes for all calls to DirectInput (via IDirectInput or IDirectInput8) and WinMM joystick (via direct function calls) APIs. In the former case, an instance of WrapperIDirectInput is returned when the application calls one of the DirectInput object creation methods exported by the Xidi DLL, and in the latter case, the joystick family of WinMM calls invokes functions supplied by WrapperJoyWinMM.

**XidiConfigReader** specializes the generic functionality offered by the Configuration subsystem and defines the allowed format of a Xidi configuration file.
