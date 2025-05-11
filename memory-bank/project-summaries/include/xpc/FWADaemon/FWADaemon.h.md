# Summary for include/xpc/FWADaemon/FWADaemon.h

This Objective-C header file declares the interface for the `FWADaemon` class. This class is the core implementation of the `FWADaemon` XPC service, responsible for handling client requests, managing shared resources like shared memory, and interacting with the FireWire audio driver (indirectly, often via client requests that originate from the driver). This file is identical in content and purpose to `include/shared/xpc/FWADaemon.h`.

**Key Declarations and Components:**

-   **`@import Foundation;`**: Imports the Foundation framework, providing base Objective-C classes and XPC support.
-   **`#import "FWADaemonControlProtocol.h"`**: Imports the Objective-C protocol that defines the interface the daemon exports to its clients. This path implies it's looking for `FWADaemonControlProtocol.h` in the same directory or a configured include path that resolves to `include/shared/xpc/FWADaemonControlProtocol.h`.

-   **`NS_ASSUME_NONNULL_BEGIN` and `NS_ASSUME_NONNULL_END`**: Standard Objective-C macros to indicate that pointers are generally non-null within this section.

-   **`extern NSString * const kFWADaemonMachServiceName;`**:
    -   Declares a constant `NSString` pointer for the Mach service name used by the daemon.
    -   The actual definition (e.g., `NSString * const kFWADaemonMachServiceName = @"net.mrmidi.FWADaemon";`) would be in the corresponding `.m` or `.mm` file (likely `FWADaemon.mm`).
    -   This is the globally unique name clients use to look up and connect to this XPC service.

-   **`@interface FWADaemon : NSObject <FWADaemonControlProtocol>`**:
    -   Declares the `FWADaemon` class.
    -   It inherits from `NSObject`, the root class for most Objective-C objects.
    -   **Protocol Conformance:** It explicitly states that it conforms to the `FWADaemonControlProtocol`. This means the `FWADaemon` class must implement all the methods declared in that protocol. These methods constitute the API that clients (like `DriverXPCManager` or `XPCBridge`) can call on the daemon.

    -   **Singleton Accessor Method:**
        -   `+ (instancetype)sharedService;`
        -   A class method that provides access to the singleton instance of the `FWADaemon`. This is a common pattern for XPC services where a single instance manages the service's state and resources.

    -   **Protocol Method Declarations (Implicit):**
        -   By conforming to `FWADaemonControlProtocol`, the `FWADaemon` class is expected to implement all methods from that protocol (e.g., client registration, shared memory name retrieval, driver status updates, control request forwarding, log forwarding). The actual implementations are in `FWADaemon.mm`.

**Overall Role:**
The `FWADaemon.h` file (in this location) defines the public class interface for the central daemon process, mirroring the one in `include/shared/xpc/`.
-   It establishes that `FWADaemon` is the object that implements the service-side logic defined by `FWADaemonControlProtocol`.
-   It provides the singleton accessor, which is the entry point for the XPC listener delegate (`FWADaemonService` in `main.m`) to get the object that will handle incoming client connections.
The core work of managing client interactions, shared memory, and inter-process communication is implemented in `src/xpc/FWADaemon/FWADaemon.mm`.
