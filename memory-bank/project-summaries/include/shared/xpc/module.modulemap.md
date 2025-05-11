# Summary for include/shared/xpc/module.modulemap

This file is a Clang module map. Its purpose is to define a module named `FWASharedXPC`. This module encapsulates the shared XPC (Cross-Process Communication) protocol definitions and related data structures that are used by both the `FWADaemon` XPC service and its clients (such as the FWA library's `XPCBridge` and the driver's `DriverXPCManager`).

**Key Declarations and Components:**

-   **`module FWASharedXPC [system] { ... }`**:
    -   This line declares a new module named `FWASharedXPC`.
    -   The `[system]` attribute indicates that this is a system module, which can influence how the compiler treats warnings from its headers and how it's linked or used.

-   **Header Files Included in the Module:**
    -   `header "FWADaemonControlProtocol.h"`:
        -   This specifies that the Objective-C protocol defining the interface that clients use to call the `FWADaemon` service is part of this module.
    -   `header "FWAClientNotificationProtocol.h"`:
        -   This specifies that the Objective-C protocol defining the interface the `FWADaemon` service uses to call back to its clients (for notifications and delegated control) is part of this module.
    -   `header "MixedAudioBuffer.h"`:
        -   Includes the definition of the `MixedAudioBuffer` Objective-C class, which is used to transfer audio data and its metadata over XPC.
    -   `header "FWADaemon.h"`:
        -   Includes the public interface declaration for the `FWADaemon` class itself. This is useful so that clients (or the daemon's own `main.m`) can refer to the `FWADaemon` class type, for instance, when setting the `exportedObject` for an `NSXPCListener`.

-   **`export *`**:
    -   This directive exports all symbols (protocols, class interfaces, constants like `kFWADaemonMachServiceName` if defined in the included headers) declared in the specified header files as part of the `FWASharedXPC` module's public interface.
    -   Clients that import this module (e.g., using `@import FWASharedXPC;` in Objective-C/Objective-C++ or `import FWASharedXPC` in Swift if a Swift overlay were provided) will have access to these exported symbols.

-   **No `link` Directive:**
    -   The absence of a `link "LibraryName"` directive suggests that this module primarily provides declarations (protocols, class interfaces, data structures) and does not, by itself, correspond to a separate linkable library target that needs to be linked against.
    -   The actual implementations of the protocols and classes are found in the `FWADaemon` executable (for the service side) and in the respective client-side libraries/components (like `libFWA_XPC.a` for the FWA library, or within the driver's user-space component).

**Overall Role:**
The `FWASharedXPC` module map provides a clean and modern way for different parts of the FWA system (daemon, FWA library, driver's user-space component) to share and use the common XPC interface definitions.
-   **Encapsulation:** It groups all related XPC protocol and data type declarations under a single module name.
-   **Reduced Global Namespace Pollution:** Helps avoid naming conflicts.
-   **Improved Build System Integration:** Facilitates easier use in projects that leverage Clang modules, especially for Swift and Objective-C interoperability.
By importing `FWASharedXPC`, different components can ensure they are using a consistent and type-safe definition of the XPC communication contract.
