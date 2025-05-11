# Summary for include/FWA/XPC/DuetXPCClientProtocol.h

This Objective-C header file defines the `DuetXPCClientProtocol`. This protocol outlines the interface that an XPC client (e.g., an application using the FWA library, or the FWA library itself acting as a client to a sub-service) must implement if it wishes to receive asynchronous callbacks or notifications from a corresponding XPC service that uses the `DuetXPCProtocol`.

**Key Declarations and Components:**

-   **`@import Foundation;`**: Imports the Foundation framework, which is essential for Objective-C development and XPC services.

-   **`@protocol DuetXPCClientProtocol <NSObject>`**:
    -   This declares an Objective-C protocol named `DuetXPCClientProtocol`.
    -   It inherits from `<NSObject>`, which is standard practice for protocols whose implementers will be Objective-C objects.
    -   **Purpose:** This protocol defines the set of methods that the XPC *service* (the one exposing `DuetXPCProtocol`) can call on its connected *clients*. It's the "client-side" or "callback" interface in a two-way XPC communication setup.
    -   **Declared Methods (Conceptual, as the provided file is empty):**
        -   The file, as shown in the project structure, is currently empty beyond the `@protocol` declaration and `@end`.
        -   If it were fully defined, it would list methods that the service might invoke on the client. Examples could include:
            ```objectivec
            // Example: Notified when a setting changes on the service side
            // - (void)settingDidChange:(NSString *)settingName toNewValue:(id)value;

            // Example: Notified of an asynchronous event
            // - (void)asynchronousEventOccurred:(NSString *)eventName details:(NSDictionary *)details;

            // Example: Service providing data back to the client asynchronously
            // - (void)didReceiveDataChunk:(NSData *)dataChunk forStreamID:(NSString *)streamID;
            ```
        -   The actual methods would depend entirely on the specific asynchronous communication needs between the "Duet" XPC service and its clients.

**Overall Role:**
The `DuetXPCClientProtocol` defines the contract for messages flowing from an XPC service (which exposes the `DuetXPCProtocol`) back to its clients.
-   When a client connects to the service, it would typically:
    1.  Set its `remoteObjectInterface` to `[NSXPCInterface interfaceWithProtocol:@protocol(DuetXPCProtocol)]`.
    2.  Create an object that conforms to `DuetXPCClientProtocol`.
    3.  Set the connection's `exportedInterface` to `[NSXPCInterface interfaceWithProtocol:@protocol(DuetXPCClientProtocol)]`.
    4.  Set the connection's `exportedObject` to the instance of its class that implements `DuetXPCClientProtocol`.
-   This setup allows the service, after receiving the client's exported object, to make calls on the methods defined in `DuetXPCClientProtocol` to send asynchronous messages or notifications back to that specific client.
-   This protocol is essential for enabling two-way communication in the "Duet" XPC interaction pattern.

**Note:** The provided file content is just the protocol declaration. The actual methods would need to be defined based on the intended interaction. This protocol seems distinct from the main `FWAClientNotificationProtocol` and `FWADaemonControlProtocol`, suggesting it might be for a different, perhaps more specialized, XPC interaction, possibly related to "Duet" display or audio features if this is part of a larger system context.
