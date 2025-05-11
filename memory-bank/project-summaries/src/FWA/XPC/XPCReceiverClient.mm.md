# Summary for src/FWA/XPC/XPCReceiverClient.mm

This Objective-C++ file implements the `FWA::XPCReceiverClient` class. This class plays a crucial role in the client-side XPC architecture by acting as the receiver for asynchronous messages and callbacks sent from the `FWADaemon` XPC service. It implements the `FWAClientNotificationProtocol`.

**Key Functionalities:**

-   **Protocol Implementation (`FWAClientNotificationProtocol`):**
    -   The primary purpose of this class is to provide concrete implementations for all methods defined in the `FWAClientNotificationProtocol`. These methods are called by the `FWADaemon` when it needs to send information or delegate actions to this specific client.
    -   **Notification Handlers:**
        -   `driverConnectionStatusDidChange:(BOOL)isConnected`: Called by the daemon when the Core Audio driver connects or disconnects. This method would typically forward this status to the `XPCBridge`, which then informs its C++ listeners.
        -   `devicePropertyChanged:(NSString *)propertyName forGUID:(uint64_t)guid value:(id)value`: Called when a property of a specific device changes (e.g., sample rate, volume, as initiated by another client or the device itself). It forwards this to `XPCBridge`.
        -   `didReceiveAudioBuffer:(MixedAudioBuffer *)buffer`: If the daemon were to stream audio back to this client (e.g., for metering in a GUI), this method would receive the `MixedAudioBuffer` object.
        -   `didReceiveLogMessageFrom:(NSString *)source level:(int32_t)level message:(NSString *)message`: Receives log messages (e.g., from the driver, forwarded by the daemon) and passes them to `XPCBridge`.
    -   **Forwarded Control Request Handlers (Implemented by the actual client, e.g., GUI):**
        -   These methods are called by the daemon when the *driver* initiates a control change that requires client-side (e.g., GUI) handling or approval. The `XPCReceiverClient` in the FWA library itself might just log these if it's not part of a GUI, or a GUI-specific subclass would implement the actual logic.
        -   `performSetNominalSampleRate:(uint64_t)guid rate:(double)rate withReply:(void (^)(BOOL success))reply;`
        -   `performSetClockSource:(uint64_t)guid clockSourceID:(uint32_t)clockSourceID withReply:(void (^)(BOOL success))reply;`
        -   `performSetMasterVolumeScalar:(uint64_t)guid scope:(uint32_t)scope element:(uint32_t)element scalarValue:(float)scalarValue withReply:(void (^)(BOOL success))reply;`
        -   `performSetMasterMute:(uint64_t)guid scope:(uint32_t)scope element:(uint32_t)element muteState:(BOOL)muteState withReply:(void (^)(BOOL success))reply;`
        -   `performStartIO:(uint64_t)guid withReply:(void (^)(BOOL success))reply;`
        -   `performStopIO:(uint64_t)guid;`
        -   The implementations of these methods would interact with the `AudioDevice` (or `DiceAudioDevice`) instance for the given `guid` to effect the change and then call the `reply` block.

-   **Anonymous Listener for Daemon Callbacks:**
    -   `init()`:
        -   Creates an anonymous `NSXPCListener` (`_listener = [NSXPCListener anonymousListener];`). This listener is not registered with a Mach service name; instead, its `endpoint` is what gets sent to the daemon.
        -   Sets `self` as the delegate for this listener (`_listener.delegate = self;`).
        -   Resumes the listener (`[_listener resume];`).
    -   `listenerEndpoint()`: Returns `_listener.endpoint`. This `NSXPCListenerEndpoint` is sent to the `FWADaemon` during client registration (`XPCBridge` calls the daemon's `registerClientWithEndpoint:`). The daemon then uses this endpoint to establish a private XPC connection back to this `XPCReceiverClient` instance.

-   **`NSXPCListenerDelegate` Conformance:**
    -   `- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection`:
        -   This delegate method is called when the `FWADaemon` connects back to this client using the provided endpoint.
        -   It configures this incoming connection:
            -   Sets `newConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(FWAClientNotificationProtocol)];` (though this is what the daemon expects the client to implement, so it's more about what the daemon *can call* on the client).
            -   Sets `newConnection.exportedObject = self;`. This makes the current `XPCReceiverClient` instance the object that will handle method calls from the daemon on this connection.
            -   Sets `newConnection.remoteObjectInterface` to `[NSXPCInterface interfaceWithProtocol:@protocol(FWADaemonControlProtocol)]`. This allows this client, if it needed to, to also make calls back to the daemon over this specific connection (though the primary client-to-daemon calls are usually via the `XPCBridge`'s main connection).
            -   Resumes the `newConnection`.

**Overall Role:**
The `XPCReceiverClient` is essential for enabling bidirectional XPC communication. While `XPCBridge` initiates the primary connection to the daemon and sends requests, `XPCReceiverClient` sets up a way for the daemon to call *back* into the client application/library. It implements the client-side of the notification protocol, receiving asynchronous updates and delegated control requests from the `FWADaemon`. These are then typically passed to the `XPCBridge`, which dispatches them to appropriate C++ listeners or handlers within the FWA library or the application using it.
