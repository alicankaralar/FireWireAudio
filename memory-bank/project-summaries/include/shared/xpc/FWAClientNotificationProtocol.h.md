# Summary for include/shared/xpc/FWAClientNotificationProtocol.h

This Objective-C header file defines the `FWAClientNotificationProtocol`. This protocol outlines the interface that the `FWADaemon` XPC service uses to send asynchronous notifications and delegate control requests (originating from the kernel driver) to its connected clients (such as a GUI application using the FWA library, or the FWA library's `XPCReceiverClient` itself).

**Key Declarations and Components:**

-   **`@import Foundation;`**: Imports the Foundation framework.
-   **Forward Declaration:** `@class MixedAudioBuffer;` (Indicates that `MixedAudioBuffer` is an Objective-C class defined elsewhere, used here as a parameter type).

-   **`@protocol FWAClientNotificationProtocol <NSObject>`**:
    -   This declares an Objective-C protocol named `FWAClientNotificationProtocol`.
    -   It inherits from `<NSObject>`, standard for protocols.
    -   **Purpose:** This protocol defines the set of methods that the `FWADaemon` (the XPC service) can call on its connected clients. It's the "client-side" or "callback" interface for the main FWA XPC communication.

    -   **Declared Methods (for Daemon-to-Client Communication):**

        -   **Status and Event Notifications:**
            -   `- (void)driverConnectionStatusDidChange:(BOOL)isConnected;`
                -   Called by the daemon to inform the client that the kernel driver's connection status (to the daemon or hardware) has changed.
            -   `- (void)devicePropertyChanged:(NSString *)propertyName forGUID:(uint64_t)guid value:(id)value;`
                -   Called when a property of a specific device (identified by `guid`) changes. `propertyName` indicates which property, and `value` is the new value (can be various Objective-C types like `NSNumber`, `NSString`).
            -   `- (void)didReceiveLogMessageFrom:(NSString *)source level:(int32_t)level message:(NSString *)message;`
                -   Called to forward log messages (e.g., from the driver or daemon itself) to the client for display or processing.
            -   `- (void)didReceiveAudioBuffer:(MixedAudioBuffer *)buffer;`
                -   Called if the daemon needs to send audio data (e.g., for metering or direct playback by a client) to the client. `MixedAudioBuffer` encapsulates the audio data and its format.

        -   **Delegated Control Requests (Driver -> Daemon -> Client [e.g., GUI]):**
            -   These methods are invoked on a client (typically a GUI client that has registered with the daemon) when the kernel driver (via Core Audio property changes) requests a change that requires user-space handling or confirmation. The client implementing these methods is expected to perform the action (e.g., by interacting with the `FWA::AudioDevice` object) and then call the `reply` block to indicate success or failure back to the daemon, which then informs the driver.
            -   `- (void)performSetNominalSampleRate:(uint64_t)guid rate:(double)rate withReply:(void (^)(BOOL success))reply;`
            -   `- (void)performSetClockSource:(uint64_t)guid clockSourceID:(uint32_t)clockSourceID withReply:(void (^)(BOOL success))reply;`
            -   `- (void)performSetMasterVolumeScalar:(uint64_t)guid scope:(uint32_t)scope element:(uint32_t)element scalarValue:(float)scalarValue withReply:(void (^)(BOOL success))reply;`
            -   `- (void)performSetMasterMute:(uint64_t)guid scope:(uint32_t)scope element:(uint32_t)element muteState:(BOOL)muteState withReply:(void (^)(BOOL success))reply;`
            -   `- (void)performStartIO:(uint64_t)guid withReply:(void (^)(BOOL success))reply;`
            -   `- (void)performStopIO:(uint64_t)guid;` (Note: `performStopIO` does not have a reply block, implying it's a one-way notification or the driver doesn't wait for confirmation for stopping).

**Overall Role:**
The `FWAClientNotificationProtocol` is essential for enabling robust, two-way communication in the FWA system's XPC architecture.
-   It defines the contract for how the `FWADaemon` can proactively send information (status updates, property changes, logs, audio data) to its connected clients.
-   Crucially, it defines how the daemon delegates control operations that originate from the kernel driver (and thus from the Core Audio system) to a user-space client (typically a GUI application). This allows the GUI to be the "source of truth" or the interactive point for certain device settings.
An object on the client side (like `FWA::XPCReceiverClient` in the FWA library, or a custom object in a GUI application) would implement this protocol and be set as the `exportedObject` on its `NSXPCConnection` to the daemon.
