# Summary for include/shared/xpc/FWADaemonControlProtocol.h

This Objective-C header file defines the `FWADaemonControlProtocol`. This protocol is crucial as it specifies the set of methods that a client (such as the user-space component of the kernel driver, `DriverXPCManager`, or the FWA library's `XPCBridge`) can call on the `FWADaemon` XPC service. It defines the "service-side" interface that the daemon implements.

**Key Declarations and Components:**

-   **`@import Foundation;`**: Imports the Foundation framework.
-   **Forward Declaration:** `@class MixedAudioBuffer;` (Though not directly used as a parameter in *this* protocol, it's often included in shared XPC protocol headers if related data types are used in the overall XPC communication scheme, or if it was intended for a method here).

-   **`NS_ASSUME_NONNULL_BEGIN` and `NS_ASSUME_NONNULL_END`**: Standard Objective-C macros.

-   **`@protocol FWADaemonControlProtocol <NSObject>`**:
    -   Declares the Objective-C protocol.
    -   **Purpose:** Defines the API exposed by the `FWADaemon` XPC service.

    -   **Declared Methods (for Client-to-Daemon Communication):**

        -   **Client Registration & Management:**
            -   `- (void)registerClient:(NSString *)clientID clientNotificationEndpoint:(NSXPCListenerEndpoint *)clientNotificationEndpoint withReply:(void (^)(BOOL success, NSDictionary * _Nullable daemonInfo))reply;`
                -   Allows a client to register with the daemon.
                -   `clientID`: A unique string identifying the client (e.g., "DriverClient", "GUIClient-AppXYZ").
                -   `clientNotificationEndpoint`: The `NSXPCListenerEndpoint` of the client, which the daemon uses to establish a connection back to the client for sending notifications (using `FWAClientNotificationProtocol`).
                -   `reply`: A block called by the daemon to indicate success/failure of registration and optionally provide initial daemon information.
            -   `- (void)unregisterClient:(NSString *)clientID;`
                -   Allows a client to unregister from the daemon.

        -   **Shared Memory and Driver Status:**
            -   `- (void)getSharedMemoryNameWithReply:(void (^)(NSString * _Nullable shmName))reply;`
                -   Called by clients (primarily the driver) to get the name of the POSIX shared memory segment used for audio data.
            -   `- (void)setDriverPresenceStatus:(BOOL)isPresent;`
                -   Called by the driver to inform the daemon whether the kernel driver is currently loaded and active.
            -   `- (void)getIsDriverConnectedWithReply:(void (^)(BOOL isConnected))reply;`
                -   Allows clients to query if the driver is currently reported as present by the daemon.

        -   **Device Information and Control Requests (Often initiated by driver, potentially forwarded to GUI by daemon):**
            -   `- (void)getDeviceConnectionStatus:(uint64_t)guid reply:(void (^)(BOOL isConnected, NSError * _Nullable error))reply;`
            -   `- (void)getDeviceConfiguration:(uint64_t)guid reply:(void (^)(NSDictionary * _Nullable config, NSError * _Nullable error))reply;` (Returns device info, likely as JSON serialized into a dictionary).
            -   `- (void)getConnectedDeviceGUIDsWithReply:(void (^)(NSArray<NSNumber *> * _Nullable guids, NSError * _Nullable error))reply;`
            -   The following `request...` methods are typically called by the driver's user-space component (`DriverXPCManager`) when Core Audio attempts to change a device property. The daemon then forwards these to a registered GUI client (if any) using the `FWAClientNotificationProtocol`'s `perform...` methods. The GUI's response is then relayed back to the driver via the `reply` block.
                -   `- (void)requestSetNominalSampleRate:(uint64_t)guid rate:(double)rate withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;`
                -   `- (void)requestSetClockSource:(uint64_t)guid clockSourceID:(uint32_t)clockSourceID withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;`
                -   `- (void)requestSetMasterVolumeScalar:(uint64_t)guid scope:(uint32_t)scope element:(uint32_t)element scalarValue:(float)scalarValue withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;`
                -   `- (void)requestSetMasterMute:(uint64_t)guid scope:(uint32_t)scope element:(uint32_t)element muteState:(BOOL)muteState withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;`
                -   `- (void)requestStartIO:(uint64_t)guid withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;`
                -   `- (void)requestStopIO:(uint64_t)guid;` (No reply block, indicating a one-way request or that the driver doesn't wait for user-space confirmation to stop I/O).

        -   **Logging:**
            -   `- (void)forwardLogMessageFromDriver:(int32_t)level message:(NSString *)message;`
                -   Called by the driver's user-space component to send log messages to the daemon, which then typically forwards them to GUI clients.

**Overall Role:**
`FWADaemonControlProtocol` defines the contract for how clients interact with the `FWADaemon` XPC service. It covers:
-   Client lifecycle management (registration).
-   Access to shared resources (shared memory name).
-   Driver status reporting and querying.
-   A mechanism for the driver to delegate control operations (that require user-space logic or confirmation) to a GUI client, with the daemon acting as the intermediary.
-   Log message forwarding.
The `FWADaemon` class (in `FWADaemon.mm`) implements these methods to provide the actual service logic.
