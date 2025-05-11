# Summary for include/FWA/XPC/XPCReceiverClient.hpp

This C++ header file declares the `FWA::XPCReceiverClient` class. This class is an Objective-C++ class (indicated by its typical usage in an `.mm` implementation file and interaction with Objective-C protocols and XPC types) designed to act as the "exported object" on the client side of an XPC connection. Specifically, it's the object within the FWA library (or an application using it) that receives asynchronous callbacks and notifications from the `FWADaemon` XPC service.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is declared within this namespace.

-   **Includes/Imports (Conceptual, as it's a .hpp for an Obj-C++ class):**
    -   While a `.hpp` file, its implementation (`.mm`) would `@import Foundation;` for `NSObject`, `NSXPCListener`, `NSXPCListenerEndpoint`, `NSXPCConnection`.
    -   It needs the definition of the `FWAClientNotificationProtocol`, so it would effectively include or be aware of `../../shared/xpc/FWAClientNotificationProtocol.h`.
    -   It needs a way to communicate back to the C++ `XPCBridge`, so a forward declaration or include of `XPCBridge.h` (or a delegate pattern) would be involved.

-   **Class Declaration (`@interface XPCReceiverClient : NSObject <FWAClientNotificationProtocol, NSXPCListenerDelegate>` - Conceptual Obj-C style for clarity):**
    -   The C++ equivalent would be `class XPCReceiverClient : public FWA::XPCNotificationListenerDelegateInterface` (if such an internal C++ interface exists for bridging) or it directly implements the logic to call `XPCBridge`.
    -   **Protocol Conformance:**
        -   It must conform to `FWAClientNotificationProtocol`. This means it will implement all the methods defined in that protocol, which are the methods the `FWADaemon` will call on this client.
        -   It also acts as an `NSXPCListenerDelegate` for its own anonymous listener.

    -   **Public Interface (Conceptual, methods exposed to `XPCBridge` or for setup):**
        -   **Constructor/Initializer:**
            -   Likely an `init` method, perhaps taking a weak pointer or reference to the `FWA::XPCBridge` instance so it can forward notifications.
            -   `XPCReceiverClient(FWA::XPCBridge* bridge);` (C++ style) or `- (instancetype)initWithBridge:(FWA::XPCBridge*)bridge;` (Obj-C style).
        -   **Listener Endpoint Accessor:**
            -   `NSXPCListenerEndpoint* listenerEndpoint() const;` (or C++ equivalent): Returns the endpoint of its internal anonymous `NSXPCListener`. This endpoint is given to the `FWADaemon` so the daemon can establish a connection back to this `XPCReceiverClient`.

    -   **`FWAClientNotificationProtocol` Method Declarations:**
        -   The header would declare (or the `.mm` file would implement) all methods from `FWAClientNotificationProtocol`. When the daemon calls these methods via XPC, the implementation in `XPCReceiverClient.mm` will:
            -   Convert Objective-C types (like `NSString`, `NSDictionary`, `NSNumber`) to their C++ equivalents (`std::string`, `nlohmann::json`, primitive types).
            -   Call the appropriate forwarding method on the `XPCBridge` instance (e.g., `_bridge->forwardDriverConnectionStatusChanged(...)`).
        -   Examples:
            -   `driverConnectionStatusDidChange:(BOOL)isConnected;`
            -   `devicePropertyChanged:(NSString *)propertyName forGUID:(uint64_t)guid value:(id)value;`
            -   `didReceiveLogMessageFrom:(NSString *)source level:(int32_t)level message:(NSString *)message;`
            -   Methods for handling forwarded control requests from the driver (e.g., `performSetNominalSampleRate:...`).

    -   **`NSXPCListenerDelegate` Method Declaration:**
        -   `- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection;`
            -   Declared here if the class directly manages its `NSXPCListener`. This method configures the incoming connection from the daemon.

    -   **Private Members (Conceptual):**
        -   `_bridge`: A raw or weak pointer to the `FWA::XPCBridge` instance.
        -   `_listener`: `NSXPCListener*` (an anonymous listener).
        -   `_acceptedConnection`: `NSXPCConnection*` (the connection established by the daemon back to this client).

**Overall Role:**
The `XPCReceiverClient` class is a critical piece of the XPC infrastructure on the client side. It acts as the "exported object" that the `FWADaemon` communicates with for asynchronous notifications and callbacks.
1.  It sets up an anonymous `NSXPCListener` to allow the daemon to connect back to it.
2.  It implements the `FWAClientNotificationProtocol`, providing the entry points for messages sent by the daemon.
3.  When it receives a message from the daemon, it typically translates any Objective-C specific data types to C++ types and then forwards the notification to the C++ `XPCBridge`.
4.  The `XPCBridge` then dispatches these notifications to any registered C++ `XPCNotificationListener` objects.
This class effectively bridges the Objective-C XPC world (where protocols are defined) with the C++ FWA library logic.
