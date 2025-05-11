# Summary for include/FWA/IOKitFireWireDeviceDiscovery.h

This C++ header file defines the `FWA::IOKitFireWireDeviceDiscovery` class. This class is a concrete implementation of the `IFireWireDeviceDiscovery` interface and is responsible for detecting the connection and disconnection of FireWire devices on the macOS system using IOKit's notification framework.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `"IFireWireDeviceDiscovery.h"`: For the base interface class and callback typedefs (`DeviceAddedCallback`, `DeviceRemovedCallback`).
    -   `<IOKit/IOKitLib.h>`: For general IOKit functions and types.
    -   `<IOKit/firewire/IOFireWireLib.h>`: Specifically for FireWire IOKit types and constants like `kIOFireWireDeviceClassName`.
    -   `<IOKit/IOMessage.h>`: For IOKit message types like `kIOFirstMatchNotification` and `kIOTerminatedNotification`.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<spdlog/spdlog.h>`: For logging.

-   **Class `IOKitFireWireDeviceDiscovery` (implements `IFireWireDeviceDiscovery`):**
    -   **Public Interface:**
        -   **Constructor:**
            -   `explicit IOKitFireWireDeviceDiscovery(std::shared_ptr<spdlog::logger> logger);`
            -   Takes a logger instance for internal logging.
        -   **Destructor:**
            -   `~IOKitFireWireDeviceDiscovery() override;`
            -   Ensures that `stop()` is called to release IOKit resources if the discovery process was active.
        -   **`IFireWireDeviceDiscovery` Interface Methods (Overrides):**
            -   `void start(DeviceAddedCallback addedCb, DeviceRemovedCallback removedCb, void* context) override;`
                -   **Implementation Details (in .cpp):**
                    -   Stores the provided `addedCb`, `removedCb`, and `context`.
                    -   Creates an `IONotificationPortRef` (`_notifyPort`).
                    -   Gets a `CFRunLoopSourceRef` from the notification port and adds it to a `CFRunLoop` (typically the one managed by `RunLoopHelper::instance().getRunLoop()`).
                    -   Creates an IOKit matching dictionary using `IOServiceMatching(kIOFireWireDeviceClassName)` to look for `IOFireWireDevice` objects.
                    -   Registers for "first match" notifications (`kIOFirstMatchNotification`) using `IOServiceAddMatchingNotification`. The static `sDeviceAdded` method is provided as the C callback, and `this` (the `IOKitFireWireDeviceDiscovery` instance) is passed as the `refCon`. An iterator (`_matchedIterator`) is returned.
                    -   Manually iterates through `_matchedIterator` once to discover already connected devices and calls `sDeviceAdded` for each.
                    -   Registers for "terminated" notifications (`kIOTerminatedNotification`) using `IOServiceAddMatchingNotification`. The static `sDeviceRemoved` method is provided as the C callback. An iterator (`_terminatedIterator`) is returned.
            -   `void stop() override;`
                -   **Implementation Details (in .cpp):**
                    -   Removes the `_runLoopSource` from the run loop.
                    -   Releases the `_matchedIterator` and `_terminatedIterator` if they are valid (`IOObjectRelease`).
                    -   Disposes of the `_notifyPort` (`IONotificationPortDestroy`).
                    -   Resets callback pointers and context to null.

    -   **Private Static Callback Trampolines:**
        -   `static void sDeviceAdded(void* refCon, io_iterator_t iterator);`
        -   `static void sDeviceRemoved(void* refCon, io_iterator_t iterator);`
        -   These C-style functions are required by IOKit. They cast `refCon` back to an `IOKitFireWireDeviceDiscovery*` and call the corresponding private instance methods.

    -   **Private Instance Callback Handlers:**
        -   `void deviceAdded(io_service_t service);`: Iterates through the `iterator` from `sDeviceAdded`, and for each `io_service_t`, calls the stored `_deviceAddedCallback`.
        -   `void deviceRemoved(io_service_t service);`: Similar to `deviceAdded`, but for device removal, calling `_deviceRemovedCallback`.

    -   **Private Members:**
        -   `std::shared_ptr<spdlog::logger> _logger;`
        -   `IONotificationPortRef _notifyPort = nullptr;`
        -   `CFRunLoopSourceRef _runLoopSource = nullptr;`
        -   `io_iterator_t _matchedIterator = IO_OBJECT_NULL;`
        -   `io_iterator_t _terminatedIterator = IO_OBJECT_NULL;`
        -   `DeviceAddedCallback _deviceAddedCallback = nullptr;`
        -   `DeviceRemovedCallback _deviceRemovedCallback = nullptr;`
        -   `void* _callbackContext = nullptr;`

**Overall Role:**
The `IOKitFireWireDeviceDiscovery` class is the concrete implementation that uses macOS's IOKit framework to detect when FireWire devices are connected to or disconnected from the system.
-   It encapsulates the details of setting up IOKit matching dictionaries and notification ports.
-   It handles the asynchronous nature of IOKit notifications by integrating with a `CFRunLoop`.
-   When a device event occurs, it translates the low-level IOKit notification into a call to the appropriate C-style callback (`DeviceAddedCallback` or `DeviceRemovedCallback`) that was provided by its client (typically the `DeviceController`).
This class is the primary means by which the FWA library becomes aware of available FireWire audio hardware.
