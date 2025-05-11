# Summary for include/FWA/DeviceController.h

This C++ header file defines the `FWA::DeviceController` class. This class serves as the main top-level manager and entry point for the FireWire Audio (FWA) library. It is responsible for discovering FireWire audio devices, managing their lifecycles, and providing access to them for client applications (either directly or via the C-API). It's implemented as a singleton.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<vector>`, `<map>`, `<memory>`, `<string>`, `<mutex>`, `<functional>`: Standard C++ library headers.
    -   `"AudioDevice.h"`: For the `AudioDevice` class.
    -   `"IFireWireDeviceDiscovery.h"`: For the device discovery interface.
    -   `"XPCBridge.h"`: For interacting with the `FWADaemon` via XPC and receiving notifications (as it implements `XPCNotificationListener`).
    -   `"Enums.hpp"`: For various enums.
    -   `"Error.h"`: For error handling.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<IOKit/IOTypes.h>`: For `io_service_t`.

-   **`DeviceListener` Interface (Conceptual, or defined elsewhere):**
    -   An interface (likely an abstract class) that clients can implement to receive notifications about device additions, removals, or property changes.
    -   Methods like `virtual void onDeviceAdded(std::shared_ptr<AudioDevice> device) = 0;`, `virtual void onDeviceRemoved(uint64_t guid) = 0;`.

-   **Class `DeviceController` (implements `XPCNotificationListener`):**
    -   **Singleton Access:**
        -   `static DeviceController& instance();`
        -   Provides global access to the single `DeviceController` instance.
    -   **Constructor/Destructor (Private/Deleted):**
        -   To enforce the singleton pattern. Copy/move operations are also deleted.
    -   **Public Interface:**
        -   **Initialization and Shutdown:**
            -   `bool init(std::unique_ptr<IFireWireDeviceDiscovery> discoveryStrategy = nullptr);`:
                -   Initializes the controller.
                -   Sets up the device discovery mechanism (e.g., creates an `IOKitFireWireDeviceDiscovery` instance if no strategy is provided).
                -   Starts device discovery (`_deviceDiscovery->start(...)`), providing internal static callbacks (`sDeviceAdded`, `sDeviceRemoved`).
                -   Connects to the `FWADaemon` via `XPCBridge::instance().connect()`.
                -   Registers itself as an `XPCNotificationListener` with `XPCBridge`.
            -   `void shutdown();`:
                -   Stops device discovery.
                -   Disconnects from `XPCBridge`.
                -   Clears managed devices and listeners.
        -   **Device Access:**
            -   `std::vector<std::shared_ptr<AudioDevice>> getConnectedDevices() const;`: Returns a list of currently connected and managed `AudioDevice` objects.
            -   `std::shared_ptr<AudioDevice> getDeviceByGUID(uint64_t guid) const;`: Retrieves a specific device by its GUID.
        -   **Listener Management (for device connect/disconnect):**
            -   `void addDeviceListener(std::weak_ptr<DeviceListener> listener);`
            -   `void removeDeviceListener(std::weak_ptr<DeviceListener> listener);`
        -   **Logging Control:**
            -   `void setLogLevel(spdlog::level::level_enum level);`

    -   **`XPCNotificationListener` Implementation (Public, called by `XPCBridge`):**
        -   `void onDriverConnectionStatusChanged(bool isConnected) override;`
        -   `void onDevicePropertyChanged(uint64_t guid, const std::string& propertyName, const nlohmann::json& value) override;`
        -   `void onLogMessageReceived(const std::string& source, int level, const std::string& message) override;`
        -   These methods handle notifications received from the `FWADaemon` and might update internal state or forward notifications to `DeviceListener`s.

    -   **Internal Device Discovery Callbacks (Static, then to instance methods):**
        -   `static void sDeviceAdded(io_service_t service, void* context);`
        -   `static void sDeviceRemoved(io_service_t service, void* context);`
        -   These call private instance methods:
            -   `void handleDeviceAdded(io_service_t service);`: Creates an `AudioDevice` (or `DiceAudioDevice` based on vendor ID), initializes it, adds it to `_managedDevices`, and notifies listeners.
            -   `void handleDeviceRemoved(io_service_t service);`: Finds the device by `io_service_t` (or its GUID), removes it from `_managedDevices`, and notifies listeners.

    -   **Private Members:**
        -   `_deviceDiscovery`: `std::unique_ptr<IFireWireDeviceDiscovery>`.
        -   `_managedDevices`: `std::map<uint64_t, std::shared_ptr<AudioDevice>>` (mapping GUID to `AudioDevice` object).
        -   `_deviceListeners`: `std::vector<std::weak_ptr<DeviceListener>>`.
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.
        -   `_xpcBridge`: A reference to `XPCBridge::instance()`.
        -   `_mutex`: `std::mutex` for thread-safe access to shared members like `_managedDevices` and `_deviceListeners`.

**Overall Role:**
The `DeviceController` is the heart of the FWA user-space library. It orchestrates:
1.  **Device Lifecycle Management:** Discovering new FireWire audio devices as they are connected, creating `AudioDevice` objects for them, initializing these objects (which involves parsing descriptors and capabilities), and removing them when they are disconnected.
2.  **Centralized Device Access:** Providing a single point for applications to get a list of connected devices or retrieve a specific device.
3.  **Event Notification:** Allowing client code to register listeners to be notified about device additions, removals, and property changes.
4.  **XPC Communication Hub:** Acting as a listener for notifications from the `FWADaemon` (via `XPCBridge`) and potentially initiating XPC communication for high-level operations.
It provides the main API surface for applications that use the FWA library directly in C++. The C-API (`fwa_capi.h`) would typically wrap the functionality of this `DeviceController`.
