# Summary for src/driver/FWADriver.cpp

This C++ file implements the `com_FWAudio_driver` class, which serves as the main entry point and core logic for the FireWire Audio kernel extension (kext). It inherits from `IOService`, the base class for IOKit drivers.

**Key Functionalities:**

-   **Driver Lifecycle Management:**
    -   `init()`: Initializes the superclass and member variables (like `_deviceInstances` dictionary).
    -   `free()`: Releases resources, including `_deviceInstances`.
    -   `start(IOService* provider)`: This is a critical method called when the kext is matched and started.
        -   Registers the service, making it available for user-space clients.
        -   **Device Matching:** Creates a matching dictionary to find `IOFireWireUnit` nubs that have an AV/C (Audio Video Control) subunit with a specific `AVCCommandSetSpecIDC` (0xA00020, likely indicating a standard audio subunit).
        -   **Device Notifications:** Registers for notifications when matching FireWire devices are published (connected) using `addMatchingNotification` for `gIOFirstMatchNotification` and `gIOTerminatedNotification`. The callbacks are `sDeviceAppeared` and `sDeviceDisappeared`.
        -   `probeDevices()`: Called to check for already connected devices that match the criteria.
        -   **XPC Initialization:** Calls `DriverXPCManager::instance().connectToDaemon()` to establish communication with the user-space daemon.
    -   `stop(IOService* provider)`: Called when the kext is being stopped.
        -   Calls `DriverXPCManager::instance().disconnectFromDaemon()`.
        -   Releases resources and calls superclass `stop`.
    -   `handleOpen()`, `handleClose()`, `handleIsOpen()`: Standard IOKit methods to manage client connections to the driver.
    -   `message(UInt32 type, IOService* provider, void* argument)`: Handles messages sent to the driver, particularly `kIOMessageServiceIsTerminated` for device disappearance.

-   **Device Discovery and Management:**
    -   `sDeviceAppeared()` (static) and `deviceAppeared()` (instance): Called when a matching FireWire device is connected.
        -   The instance method creates a new `FWADriverDevice` object to manage this specific device.
        -   It initializes the `FWADriverDevice` and attaches it to this `com_FWAudio_driver` instance.
        -   Stores the `FWADriverDevice` instance in the `_deviceInstances` dictionary, keyed by the `IOFireWireUnit` service object.
    -   `sDeviceDisappeared()` (static) and `deviceDisappeared()` (instance): Called when a FireWire device is disconnected.
        -   Retrieves the corresponding `FWADriverDevice` from `_deviceInstances`.
        -   Terminates and releases the `FWADriverDevice`.
        -   Removes it from the dictionary.
    -   `probeDevices()`: Iterates through existing `IOFireWireUnit` services at startup to find any already connected matching devices and calls `deviceAppeared()` for them.

-   **User Client Creation:**
    -   `newUserClient(task_t owningTask, void* securityID, UInt32 type, IOUserClient** handler)`: This method is called by IOKit when a user-space process attempts to open a connection to the driver.
        -   It's responsible for creating an instance of the driver's user client class (which is likely `FWADriverUserClient`, although its definition is not in this file).
        -   The `type` parameter can be used to differentiate between different types of user clients if the driver supports multiple interfaces (e.g., one for Core Audio, another for direct control).
        -   The created user client object is returned via the `handler` parameter.

-   **Internal State:**
    -   `_deviceInstances`: An `OSDictionary` to store active `FWADriverDevice` objects, mapping the `IOFireWireUnit` service to its `FWADriverDevice` handler.
    -   Notification ports and iterators for device matching.

**Overall Role:**
The `com_FWAudio_driver` class is the central orchestrator for the kext. It doesn't handle audio data directly but is responsible for:
1.  Detecting the connection and disconnection of compatible FireWire audio devices.
2.  Creating and managing `FWADriverDevice` instances, one for each connected audio device. These `FWADriverDevice` objects will then handle the specifics of interacting with that particular device.
3.  Providing an entry point for user-space clients (like the Core Audio HAL plugin) to connect to the driver via user clients.
4.  Initiating and terminating the XPC connection to the user-space daemon.
