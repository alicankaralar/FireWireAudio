# Summary for src/FWA/XPC/XPCBridge.mm

This Objective-C++ file implements the `FWA::XPCBridge` class. This class serves as the client-side component within the FWA user-space library for establishing and managing XPC (Cross-Process Communication) with the `FWADaemon` XPC service. It's implemented as a singleton.

**Key Functionalities:**

-   **Singleton Pattern (`instance()`):**
    -   Provides a static `instance()` method for global access.

-   **XPC Connection Management (`connect`, `disconnect`, `isConnected`):**
    -   `connect()`:
        -   If not already connected, it creates an `NSXPCConnection` to the `FWADaemon` service using its Mach service name (`kFWADaemonMachServiceName`, which is "net.mrmidi.FWADaemon").
        -   **Remote Interface (Daemon's Protocol):** Sets `_connection.remoteObjectInterface` to `[NSXPCInterface interfaceWithProtocol:@protocol(FWADaemonControlProtocol)]`. This tells the XPC system what methods the client (this bridge) can call on the daemon.
        -   **Exported Object (Client's Protocol for Callbacks):**
            -   Creates an instance of `XPCReceiverClient` (`_receiverClient`), which implements the `FWAClientNotificationProtocol`.
            -   Sets `_connection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(FWAClientNotificationProtocol)]`.
            -   Sets `_connection.exportedObject = _receiverClient`. This allows the daemon to make calls back to this client (e.g., for notifications).
        -   **Interruption and Invalidation Handlers:** Sets up blocks to handle connection interruptions (e.g., daemon crash) and invalidations (e.g., daemon quits). These handlers typically log the event and attempt to reconnect.
        -   Resumes the connection (`[_connection resume]`).
        -   Calls `registerClientWithDaemon()` after establishing the connection.
    -   `disconnect()`: Invalidates the `_connection` and cleans up.
    -   `isConnected()`: Returns the current connection status.

-   **Client Registration with Daemon:**
    -   `registerClientWithDaemon()`:
        -   Called after a successful connection.
        -   It retrieves the listener endpoint from the `_receiverClient` (which would have set up its own `NSXPCListener` to receive callbacks from the daemon).
        -   It then calls `[_daemonProxy registerClient:clientID clientNotificationEndpoint:endpoint withReply:...]` on the daemon's remote object proxy. This two-way registration allows the daemon to communicate back to this specific client instance.

-   **Sending Requests to Daemon (Wrapping `FWADaemonControlProtocol` methods):**
    -   The `XPCBridge` provides methods that mirror the `FWADaemonControlProtocol`. When these methods are called, they dispatch the call to the `_daemonProxy` (the remote object representing the daemon).
    -   Examples:
        -   `getDaemonInfo(...)`
        -   `getSharedMemoryName(...)`
        -   `getIsDriverConnected(...)`
        -   `requestSetNominalSampleRate(guid, rate, reply)`
        -   `requestSetClockSource(guid, clockSourceID, reply)`
        -   `requestStartIO(guid, reply)`, `requestStopIO(guid)`
        -   `getDeviceConnectionStatus(guid, reply)`, `getDeviceConfiguration(guid, reply)`
        -   `getConnectedDeviceGUIDs(reply)`
    -   These methods typically take a C++ `std::function` as a reply handler, which is invoked when the daemon sends back the result.

-   **Notification Handling (via `XPCReceiverClient`):**
    -   The `_receiverClient` object (implementing `FWAClientNotificationProtocol`) receives asynchronous messages from the daemon.
    -   `XPCBridge` manages a list of listeners (`_listeners`) that conform to `XPCNotificationListener` (a C++ interface).
    -   When `XPCReceiverClient` receives a notification (e.g., `driverConnectionStatusDidChange:`, `devicePropertyChanged:forGUID:`, `didReceiveLogMessageFrom:level:message:`), it forwards these to the `XPCBridge`.
    -   The `XPCBridge` then iterates through its registered `_listeners` and invokes the corresponding method on each listener (e.g., `onDriverConnectionStatusChanged`, `onDevicePropertyChanged`). This is done on a dedicated dispatch queue (`_notificationQueue`) for thread safety.

-   **Thread Safety:**
    -   Uses a `dispatch_queue_t _xpcQueue` (serial) for XPC connection management and outgoing calls to ensure thread safety.
    -   Uses a `dispatch_queue_t _notificationQueue` for delivering notifications to listeners.

**Overall Role:**
The `FWA::XPCBridge` is the central XPC client component for the FWA user-space library. It encapsulates all the complexities of:
1.  Establishing and maintaining a robust XPC connection to the `FWADaemon`.
2.  Performing the two-way registration required for the daemon to send callbacks/notifications to this client.
3.  Providing a C++-friendly API for other parts of the FWA library to make synchronous or asynchronous requests to the daemon.
4.  Receiving asynchronous notifications from the daemon (via `XPCReceiverClient`) and dispatching them to interested C++ listeners within the FWA library.
It acts as the crucial link between the higher-level FWA library logic and the `FWADaemon` service.
