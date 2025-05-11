# Summary for src/xpc/FWADaemon/FWADaemon.mm

This Objective-C++ file implements the `FWADaemon` class, which is the core logic for the XPC service named "net.mrmidi.FWADaemon". This daemon acts as a central intermediary between the kernel-level driver, user-space FWA library clients (like a GUI application), and potentially other components.

**Key Functionalities:**

-   **Singleton Implementation (`+ (instancetype)sharedService`):**
    -   Ensures only one instance of `FWADaemon` exists, managing shared resources and state.

-   **Initialization (`- (instancetype)init`):**
    -   Creates a serial dispatch queue (`_internalQueue`) for thread-safe operations.
    -   Initializes a dictionary (`_connectedClients`) to store information about connected XPC clients.
    -   Sets the initial `_driverIsConnected` status to `NO`.
    -   Defines and stores the shared memory name (`_sharedMemoryName` from `kSharedMemoryName = "/fwa_daemon_shm_v1"`).
    -   **Crucially, calls `[self setupSharedMemory];` to create or open the POSIX shared memory segment used for audio data transfer with the driver.**

-   **Shared Memory Setup (`- (BOOL)setupSharedMemory`):**
    -   Uses POSIX shared memory functions (`shm_open`, `ftruncate`).
    -   Attempts to create the shared memory segment exclusively. If it already exists, it opens the existing one.
    -   Sets the size of the segment to `sizeof(RTShmRing::SharedRingBuffer_POD)`.
    -   Calls `RingBufferManager::instance().map(fd, isCreator)` to map the shared memory into the daemon's address space. The `RingBufferManager` (implemented in `RingBufferManager.mm`) handles the `mmap` call and, if `isCreator` is true, initializes the `ControlBlock_POD` within the shared memory (e.g., version, capacity, zeroing read/write counts).

-   **Client Registration and Management (`FWADaemonControlProtocol` methods):**
    -   `- (void)registerClient:(NSString *)clientID clientNotificationEndpoint:(NSXPCListenerEndpoint *)clientNotificationEndpoint withReply:(void (^)(BOOL, NSDictionary *))reply`:
        -   This method is called by clients (e.g., `XPCBridge` on behalf of the FWA library or GUI, and `DriverXPCManager` from the driver's user-space component) to register with the daemon.
        -   It takes a `clientID` and an `NSXPCListenerEndpoint` from the client.
        -   The daemon uses this `clientNotificationEndpoint` to establish a new `NSXPCConnection` *back to the registering client*. This allows the daemon to send asynchronous notifications/callbacks to that client.
        -   The interface for this callback connection is set to `FWAClientNotificationProtocol`.
        -   A `ClientInfo` object (storing the clientID, the connection back to the client, and the remote proxy for that connection) is created and stored in the `_connectedClients` dictionary.
        -   Handles re-registration by invalidating any old connection for the same `clientID`.
    -   `- (void)unregisterClient:(NSString *)clientID`:
        -   Invalidates the connection to the specified client and removes it from the registry.

-   **Core `FWADaemonControlProtocol` Implementations:**
    -   `- (void)getSharedMemoryNameWithReply:(void (^)(NSString *))reply`:
        -   Returns the name of the shared memory segment (`_sharedMemoryName`) to the calling client (this is primarily used by the driver).
    -   `- (void)setDriverPresenceStatus:(BOOL)isPresent`:
        -   Called by the driver (via `DriverXPCManager`) to inform the daemon whether the driver is loaded/active.
        -   Updates the internal `_driverIsConnected` flag.
        -   Calls `broadcastDriverConnectionStatus:` to notify registered GUI clients.
    -   `- (void)getIsDriverConnectedWithReply:(void (^)(BOOL))reply`:
        -   Allows clients to query the current driver connection status.
    -   **Control Command Forwarding (Driver -> Daemon -> GUI):**
        -   A set of methods like `requestSetNominalSampleRate:rate:withReply:`, `requestSetClockSource:clockSourceID:withReply:`, `requestStartIO:withReply:`, etc.
        -   These methods are called by the driver when a Core Audio property change requires user-space (potentially GUI) intervention or confirmation.
        -   The daemon identifies a "GUI" client (by checking if `clientID` starts with "GUI").
        -   It then forwards the request by calling the corresponding method on that GUI client's remote proxy (which conforms to `FWAClientNotificationProtocol`).
        -   The reply block from the GUI client is then used to reply back to the original caller (the driver).
    -   `- (void)forwardLogMessageFromDriver:(int32_t)level message:(NSString *)message`:
        -   Receives log messages from the driver and forwards them to all connected GUI clients by calling `didReceiveLogMessageFrom:level:message:` on their remote proxies.

-   **Broadcasting and Notification:**
    -   `- (void)broadcastDriverConnectionStatus:(BOOL)isConnected`:
        -   Iterates through all connected clients. If a client's ID starts with "GUI", it sends the `driverConnectionStatusDidChange:isConnected` message to that client's remote proxy.

-   **Thread Safety:**
    -   Uses `performOnInternalQueueSync:` and `performOnInternalQueueAsync:` helper methods to ensure that modifications to shared state (like `_connectedClients` and `_driverIsConnected`) are performed serially on the `_internalQueue`.

**Overall Role:**
The `FWADaemon` is a central XPC service that acts as a crucial communication and coordination hub. Its responsibilities include:
1.  **Shared Memory Orchestration:** Creating and providing access to the shared memory segment used for low-latency audio data transfer between the kernel driver and user-space (specifically, the daemon itself via `RingBufferManager`).
2.  **Client Registry:** Managing connections from various clients (driver, GUI applications using the FWA library).
3.  **Status Brokering:** Receiving status updates from the driver (e.g., presence) and broadcasting them to interested clients (e.g., GUIs).
4.  **Control Command Delegation:** Receiving control requests from the driver (that originate from Core Audio) and delegating them to a registered GUI client for handling, then relaying the GUI's response back to the driver. This allows for user interaction or complex user-space logic to be involved in device control.
5.  **Log Aggregation/Forwarding:** Collecting logs from the driver and forwarding them to GUIs.
