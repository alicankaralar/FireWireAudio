# Summary for include/FWA/IFireWireDeviceDiscovery.h

This C++ header file defines the `FWA::IFireWireDeviceDiscovery` class, which serves as an abstract interface (a contract) for FireWire device discovery mechanisms. Any class that aims to provide device discovery functionality for the FWA library must implement this interface.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The interface is defined within the `FWA` namespace.

-   **Includes:**
    -   `<IOKit/IOTypes.h>`: For the `io_service_t` type, which is an IOKit handle representing a system service (in this context, a FireWire device).

-   **Callback Function Pointer Typedefs:**
    -   `typedef void (*DeviceAddedCallback)(io_service_t service, void* context);`
        -   Defines the signature for a C-style callback function that will be invoked when a new FireWire device is discovered (or an existing one is enumerated at startup).
        -   `service`: The IOKit service object representing the added device.
        -   `context`: A void pointer for user-defined context, allowing the registrant of the callback (e.g., `DeviceController`) to pass a `this` pointer or other relevant data.
    -   `typedef void (*DeviceRemovedCallback)(io_service_t service, void* context);`
        -   Defines the signature for a C-style callback function that will be invoked when a previously discovered FireWire device is removed (disconnected).
        -   Parameters are similar to `DeviceAddedCallback`.

-   **Interface Class `IFireWireDeviceDiscovery`:**
    -   **Virtual Destructor:**
        -   `virtual ~IFireWireDeviceDiscovery() = default;`
        -   A virtual destructor is crucial for any class intended to be used as a base class in C++, ensuring proper cleanup when derived objects are deleted through a base class pointer.
    -   **Pure Virtual Methods (Abstract Methods):**
        -   These methods define the contract that concrete discovery classes must implement.
        -   `virtual void start(DeviceAddedCallback addedCb, DeviceRemovedCallback removedCb, void* context) = 0;`
            -   **Purpose:** To initiate the device discovery process.
            -   **Parameters:**
                -   `addedCb`: A function pointer to the callback that should be invoked when a device is added.
                -   `removedCb`: A function pointer to the callback that should be invoked when a device is removed.
                -   `context`: User-defined context to be passed to the callbacks.
            -   The implementing class is responsible for setting up the necessary system mechanisms (e.g., IOKit notifications) to detect device arrivals and departures and then invoking the provided callbacks.
        -   `virtual void stop() = 0;`
            -   **Purpose:** To terminate the device discovery process and release any resources used by the discovery mechanism (e.g., unregistering notifications, closing ports).

**Overall Role:**
The `IFireWireDeviceDiscovery` interface defines a clear and abstract contract for how device discovery should operate within the FWA library.
-   **Decoupling:** It decouples the `DeviceController` (which needs device discovery services) from the specific implementation of how that discovery is performed (e.g., whether it's via IOKit, a mock for testing, or potentially another OS's native APIs if the library were ported).
-   **Strategy Pattern:** This allows different discovery strategies to be plugged into the `DeviceController`. For instance, `IOKitFireWireDeviceDiscovery` is one concrete implementation of this interface.
-   **Testability:** Makes it easier to test components that depend on device discovery by allowing mock implementations of this interface to be injected.
The `DeviceController` would hold a pointer (likely a `std::unique_ptr`) to an `IFireWireDeviceDiscovery` object and use its `start` and `stop` methods to manage the discovery lifecycle.
