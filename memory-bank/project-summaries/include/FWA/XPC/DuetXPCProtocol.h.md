# Summary for include/FWA/XPC/DuetXPCProtocol.h

This Objective-C header file defines the `DuetXPCProtocol`. This protocol outlines the interface that an XPC *service* (presumably related to "Duet" functionality, which often pertains to using an iPad as a secondary display or input device, or similar inter-device features) exposes to its clients. Clients will use this protocol to make requests to the service.

**Key Declarations and Components:**

-   **`@import Foundation;`**: Imports the Foundation framework, necessary for Objective-C and XPC types.

-   **`@protocol DuetXPCProtocol <NSObject>`**:
    -   This declares an Objective-C protocol named `DuetXPCProtocol`.
    -   It inherits from `<NSObject>`, standard for protocols implemented by Objective-C objects.
    -   **Purpose:** This protocol defines the set of methods that clients can call *on the XPC service*. It's the "service-side" interface.
    -   **Declared Methods (Conceptual, as the provided file is empty):**
        -   The file, as shown in the project structure, is currently empty beyond the `@protocol` declaration and `@end`.
        -   If it were fully defined, it would list methods representing the services offered. Examples could include:
            ```objectivec
            // Example: Requesting the service to perform an action
            // - (void)performDuetAction:(NSString *)actionName
            //              withParameters:(NSDictionary *)parameters
            //                       reply:(void (^)(BOOL success, NSError * _Nullable error))reply;

            // Example: Querying status or data from the service
            // - (void)queryDuetStatusForKey:(NSString *)statusKey
            //                         reply:(void (^)(id _Nullable value, NSError * _Nullable error))reply;

            // Example: Configuring a Duet feature
            // - (void)configureDuetFeature:(NSString *)featureName
            //                     enabled:(BOOL)isEnabled
            //                       reply:(void (^)(NSError * _Nullable error))reply;
            ```
        -   The actual methods would be specific to the functionalities provided by this "Duet" XPC service. Many methods in XPC protocols include a `reply` block for asynchronous responses.

**Overall Role:**
The `DuetXPCProtocol` defines the contract for messages flowing from a client to the "Duet" XPC service.
-   When a client (e.g., an application or another part of the FWA system) wants to interact with this service, it would:
    1.  Establish an `NSXPCConnection` to the service (using the service's Mach name).
    2.  Set the connection's `remoteObjectInterface` to `[NSXPCInterface interfaceWithProtocol:@protocol(DuetXPCProtocol)]`.
    3.  Obtain a remote object proxy using `[_connection remoteObjectProxyWithErrorHandler:]` or `[_connection synchronousRemoteObjectProxyWithErrorHandler:]`.
    4.  Call the methods defined in `DuetXPCProtocol` on this proxy.
-   This protocol is the counterpart to `DuetXPCClientProtocol`, which defines messages from the service back to the client. Together, they enable two-way XPC communication.

**Note:** The provided file content is just the protocol declaration. The actual methods would need to be defined based on the intended interaction. This protocol seems distinct from the main `FWAClientNotificationProtocol` and `FWADaemonControlProtocol`, suggesting it might be for a different, perhaps more specialized, XPC interaction, possibly related to "Duet" display or audio features if this is part of a larger system context involving inter-device continuity features.
