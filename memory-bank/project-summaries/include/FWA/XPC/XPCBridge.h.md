# Summary for include/FWA/XPC/XPCBridge.h

This C++ header file defines the `FWA::XPCBridge` class and the `FWA::XPCNotificationListener` interface. The `XPCBridge` is a singleton class that acts as the primary client-side component for managing XPC (Cross-Process Communication) with the `FWADaemon` service. The `XPCNotificationListener` interface allows other C++ components to receive asynchronous notifications from the daemon via the bridge.

**Key Declarations and Components:**

-   **Namespace `FWA`:** All definitions are within this namespace.

-   **Includes:**
    -   `<Foundation/NSXPCConnection.h>` (via Objective-C context, or forward declarations if strictly C++ header).
    -   `"../../shared/xpc/FWADaemonControlProtocol.h"`: The Objective-C protocol defining methods the client can call on the daemon.
    -   `"../../shared/xpc/FWAClientNotificationProtocol.h"`: The Objective-C protocol defining methods the daemon can call on the client (for notifications/callbacks).
    -   `"../Error.h"`: For `IOKitError`.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<nlohmann/json.hpp>`: For JSON data types, if used in notifications.
    -   Standard C++ headers: `<functional>`, `<memory>`, `<string>`, `<vector>`, `<mutex>`, `<atomic>`.

-   **Interface `XPCNotificationListener` (Abstract Class):**
    -   `class XPCNotificationListener { ... };`
    -   **Purpose:** Defines a contract for objects that want to receive asynchronous notifications from the `FWADaemon` (forwarded by `XPCBridge`).
    -   **Pure Virtual Methods:**
        -   `virtual void onDriverConnectionStatusChanged(bool isConnected) = 0;`: Called when the driver's connection status to the daemon changes.
        -   `virtual void onDevicePropertyChanged(uint64_t guid, const std::string& propertyName, const nlohmann::json& value) = 0;`: Called when a property of a specific device changes.
        -   `virtual void onLogMessageReceived(const std::string& source, int level, const std::string& message) = 0;`: Called when a log message is forwarded from the daemon.
        -   **(Potentially methods for handling forwarded control requests if this listener is part of a GUI client that needs to respond to driver-initiated changes):**
            -   `virtual void onPerformSetNominalSampleRate(uint64_t guid, double rate, std::function<void(bool success)> reply) = 0;`
            -   And similar for other `perform...` methods in `FWAClientNotificationProtocol`.

-   **Class `XPCBridge`:**
    -   **Singleton Access:**
        -   `static XPCBridge& instance();`
    -   **Constructor/Destructor (Private/Deleted):**
        -   `XPCBridge(); ~XPCBridge();`
        -   Copy/move constructors and assignment operators are deleted to enforce singleton.
    -   **Public Connection Management:**
        -   `bool connect();`: Attempts to establish the XPC connection to the `FWADaemon`.
        -   `void disconnect();`: Disconnects from the daemon.
        -   `bool isConnected() const;`: Checks the current connection status.
    -   **Public Daemon Interaction Methods (Wrapping `FWADaemonControlProtocol`):**
        -   These methods provide a C++ API to call methods on the remote `FWADaemon` object. They typically take a `std::function` as a reply handler for asynchronous responses.
        -   `void getDaemonInfo(std::function<void(NSDictionary*, NSError*)> reply) const;`
        -   `void getSharedMemoryName(std::function<void(NSString*, NSError*)> reply) const;`
        -   `void getIsDriverConnected(std::function<void(BOOL, NSError*)> reply) const;`
        -   `void requestSetNominalSampleRate(uint64_t guid, double rate, std::function<void(bool, NSError*)> reply) const;`
        -   (Other methods mirroring `FWADaemonControlProtocol` for device control, getting device lists, etc.)
    -   **Listener Management:**
        -   `void addListener(std::weak_ptr<XPCNotificationListener> listener);`
        -   `void removeListener(std::weak_ptr<XPCNotificationListener> listener);`
    -   **Internal Forwarding (from `XPCReceiverClient` - Conceptual):**
        -   Methods (likely private or protected, called by its internal `XPCReceiverClient` instance) to receive notifications from the Objective-C side and dispatch them to registered C++ `XPCNotificationListener`s.
            -   `void forwardDriverConnectionStatusChanged(bool isConnected);`
            -   `void forwardDevicePropertyChanged(uint64_t guid, const std::string& propertyName, const nlohmann::json& value);`
            -   `void forwardLogMessage(const std::string& source, int level, const std::string& message);`

    -   **Private Members (Conceptual - implementation in .mm):**
        -   `_connection`: `NSXPCConnection*` (the actual XPC connection object).
        -   `_daemonProxy`: `id<FWADaemonControlProtocol>` (the remote object proxy for calling the daemon).
        -   `_receiverClient`: An instance of `XPCReceiverClient` (Objective-C++ class implementing `FWAClientNotificationProtocol` and acting as the `exportedObject`).
        -   `_listeners`: `std::vector<std::weak_ptr<XPCNotificationListener>>`.
        -   `_listenerMutex`: `std::mutex` to protect `_listeners`.
        -   `_xpcQueue`: `dispatch_queue_t` for serializing XPC operations.
        -   `_notificationQueue`: `dispatch_queue_t` for dispatching notifications to listeners.
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.

**Overall Role:**
The `XPCBridge` class is the central nervous system for communication between the C++ FWA library (and applications using it) and the Objective-C/Swift `FWADaemon` XPC service.
1.  It manages the lifecycle of the XPC connection.
2.  It provides a C++-friendly API to send commands/requests to the daemon.
3.  It receives asynchronous notifications from the daemon (via its internal `XPCReceiverClient`) and dispatches these to registered C++ `XPCNotificationListener` objects.
This allows for a clean separation between the C++ FWA library logic and the Objective-C/XPC specifics, enabling inter-process communication for device control, status updates, and shared memory coordination.
