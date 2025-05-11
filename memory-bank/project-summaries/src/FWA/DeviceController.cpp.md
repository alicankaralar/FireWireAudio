# Summary for src/FWA/DeviceController.cpp

This C++ file implements the `FWA::DeviceController` class, which acts as a central manager for FireWire audio devices within the user-space FWA library. It is implemented as a singleton and is responsible for device discovery, lifecycle management, and notifying interested listeners about device changes.

**Key Functionalities:**

-   **Singleton Pattern (`instance()`):**
    -   Provides a static `instance()` method for global access to the single `DeviceController` object.

-   **Device Discovery (`startDiscovery`, `stopDiscovery`):**
    -   `startDiscovery()`:
        -   Creates an instance of `IFireWireDeviceDiscovery` (concretely, `IOKitFireWireDeviceDiscovery`).
        -   Registers static callback functions (`sDeviceAddedCb`, `sDeviceRemovedCb`) with the discovery object. These C-style callbacks will be invoked by `IOKitFireWireDeviceDiscovery` when FireWire devices are connected or disconnected. The `this` pointer of the `DeviceController` is passed as the context.
        -   Calls `start()` on the discovery object to begin monitoring for device events.
    -   `stopDiscovery()`: Stops the discovery process and releases the discovery object.

-   **Device Management Callbacks (Static and Instance):**
    -   `sDeviceAddedCb(io_service_t service, void* refCon)` and `sDeviceRemovedCb(io_service_t service, void* refCon)`:
        -   Static C-style callbacks. They cast `refCon` to `DeviceController*` and call the corresponding instance methods (`deviceAdded`, `deviceRemoved`).
    -   `deviceAdded(io_service_t service)`:
        -   This method is called when a new FireWire device is detected.
        -   It creates an `IOFireWireLibDeviceRef` from the `io_service_t`.
        -   Retrieves the device's GUID, vendor ID, and model ID.
        -   **DICE Device Detection:** It checks if the device is a DICE-based device by comparing vendor/model IDs against known DICE IDs (e.g., TC Electronic, Behringer, Midas that use DICE chips).
        -   **Device Instantiation:**
            -   If it's a DICE device, it creates a `std::shared_ptr<DiceAudioDevice>`.
            -   Otherwise, it creates a `std::shared_ptr<AudioDevice>`.
        -   Calls `init()` on the newly created device object (which involves parsing descriptors, etc.).
        -   Stores the device object in a `std::map<uint64_t, std::shared_ptr<AudioDevice>> _devices`, keyed by the device GUID.
        -   Calls `notifyDeviceAdded()` to inform registered listeners.
    -   `deviceRemoved(io_service_t service)`:
        -   Called when a device is disconnected.
        -   Finds the corresponding `AudioDevice` in the `_devices` map using its service object (or GUID, if tracked).
        -   Calls `notifyDeviceRemoved()` before removing it from the map and releasing it.

-   **Listener Notification System:**
    -   `addListener(std::weak_ptr<DeviceListener> listener)`: Allows other objects (that implement the `DeviceListener` interface, which would have `onDeviceAdded` and `onDeviceRemoved` methods) to register for device notifications. Uses `std::weak_ptr` to avoid circular dependencies.
    -   `removeListener(std::weak_ptr<DeviceListener> listener)`: Unregisters a listener.
    -   `notifyDeviceAdded(std::shared_ptr<AudioDevice> device)`: Iterates through the list of listeners and calls `onDeviceAdded(device)` on each valid listener.
    -   `notifyDeviceRemoved(std::shared_ptr<AudioDevice> device)`: Iterates and calls `onDeviceRemoved(device)`.

-   **Device Access:**
    -   `getDevice(uint64_t guid) const`: Returns a `std::shared_ptr<AudioDevice>` for the given GUID if found, otherwise `nullptr`.
    -   `getAllDevices() const`: Returns a `std::vector` of all currently known/connected `AudioDevice` objects.

-   **XPC Interaction (Implied/Potential):**
    -   While not directly shown in the `DeviceController` snippet, in a system with a separate daemon, this controller (or the `AudioDevice` objects it creates) might interact with an XPC client (like `XPCBridge`) to communicate with the daemon for operations that require elevated privileges or coordination with the driver (e.g., initiating isochronous streaming, getting shared memory details).

**Overall Role:**
The `DeviceController` is the central point in the FWA user-space library for managing FireWire audio devices. It abstracts the complexities of IOKit's device notification system and provides a clean interface for applications or other library components to:
1.  Be notified when FireWire audio devices are connected or disconnected.
2.  Obtain `AudioDevice` (or `DiceAudioDevice`) objects to interact with specific devices.
3.  List all available devices.
Its ability to differentiate and create specialized `DiceAudioDevice` instances allows for handling DICE-specific features.
