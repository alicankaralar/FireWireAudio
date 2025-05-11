# Summary for include/shared/xpc/FWADaemon.h

This Objective-C header file declares the interface for the `FWADaemon` class. This class is the core implementation of the `FWADaemon` XPC service, responsible for handling client requests, managing shared resources like shared memory, and interacting with the FireWire audio driver (indirectly, often via client requests that originate from the driver).

**Key Declarations and Components:**

-   **`@import Foundation;`**: Imports the Foundation framework, providing base Objective-C classes and XPC support.
-   **`#import "FWADaemonControlProtocol.h"`**: Imports the Objective-C protocol that defines the interface the daemon exports to its clients. This means the `FWADaemon` class will conform to and implement the methods declared in this protocol.

-   **`NS_ASSUME_NONNULL_BEGIN` and `NS_ASSUME_NONNULL_END`**: Standard Objective-C macros to indicate that pointers are generally non-null within this section, aiding in Swift interoperability and static analysis.

-   **`extern NSString * const kFWADaemonMachServiceName;`**:
    -   Declares a constant `NSString` pointer for the Mach service name used by the daemon.
    -   The actual definition (e.g., `NSString * const kFWADaemonMachServiceName = @"net.mrmidi.FWADaemon";`) would be in the corresponding `.m` or `.mm` file.
    -   This is the globally unique name clients use to look up and connect to this XPC service.

-   **`@interface FWADaemon : NSObject <FWADaemonControlProtocol>`**:
    -   Declares the `FWADaemon` class.
    -   It inherits from `NSObject`, the root class for most Objective-C objects.
    -   **Protocol Conformance:** It explicitly states that it conforms to the `FWADaemonControlProtocol`. This means the `FWADaemon` class must implement all the methods declared in that protocol. These methods constitute the API that clients (like `DriverXPCManager` or `XPCBridge`) can call on the daemon.

    -   **Singleton Accessor Method:**
        -   `+ (instancetype)sharedService;`
        -   A class method that provides access to the singleton instance of the `FWADaemon`. This is a common pattern for XPC services where a single instance manages the service's state and resources.

    -   **Protocol Method Declarations (Implicit):**
        -   While not explicitly re-declared in this header, by conforming to `FWADaemonControlProtocol`, the `FWADaemon` class is expected to implement all methods from that protocol. These would include:
            -   Client registration/unregistration methods (e.g., `registerClient:clientNotificationEndpoint:withReply:`).
            -   Methods for querying daemon status or shared resources (e.g., `getSharedMemoryNameWithReply:`, `getIsDriverConnectedWithReply:`).
            -   Methods for receiving status updates from the driver (e.g., `setDriverPresenceStatus:`).
            -   Methods for handling control requests forwarded from the driver to a GUI client (e.g., `requestSetNominalSampleRate:rate:withReply:`).
            -   Log forwarding methods (e.g., `forwardLogMessageFromDriver:message:`).
        -   The actual implementations of these methods are found in `FWADaemon.mm`.

    -   **Internal Properties and Methods (Not declared in the public header but exist in the implementation):**
        -   Management of connected client information (e.g., a dictionary of client connections).
        -   Logic for setting up and managing the POSIX shared memory segment.
        -   State variables (e.g., `_driverIsConnected`).
        -   Dispatch queues for thread-safe operations.

**Overall Role:**
The `FWADaemon.h` file defines the public class interface for the central daemon process.
-   It establishes that `FWADaemon` is the object that implements the service-side logic defined by `FWADaemonControlProtocol`.
-   It provides the singleton accessor, which is the entry point for the XPC listener delegate (`FWADaemonService` in `main.m`) to get the object that will handle incoming client connections.
The core work of managing client interactions, shared memory, and inter-process communication is implemented in `FWADaemon.mm`.
