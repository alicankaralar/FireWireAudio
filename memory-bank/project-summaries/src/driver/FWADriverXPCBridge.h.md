# Summary for src/driver/FWADriverXPCBridge.h

This C header file defines a C-style interface for the `FWADriverXPCBridge` class, which is implemented in Objective-C++ (`FWADriverXPCBridge.mm`). This bridge serves as an intermediary, allowing the C++ components of the kernel driver (like `FWADriverDevice`) to interact with the XPC communication logic (managed by `DriverXPCManager`) without needing to directly include or understand Objective-C/C++ specifics.

**Key Declarations:**

-   **`extern "C"`:** Ensures C linkage for the declared functions.

-   **`FWADriverXPCBridgeHandle` (Opaque Pointer):**
    -   `typedef void* FWADriverXPCBridgeHandle;`
    -   Defines an opaque pointer type. C++ code will hold this handle to refer to an instance of the underlying Objective-C++ `FWADriverXPCBridgeImpl` object.

-   **Lifecycle Functions:**
    -   `FWADriverXPCBridgeHandle FWADriverXPCBridge_Create(void);`:
        -   Creates an instance of the `FWADriverXPCBridgeImpl` object.
        -   Returns an opaque handle to it.
    -   `void FWADriverXPCBridge_Destroy(FWADriverXPCBridgeHandle handle);`:
        -   Destroys the bridge instance associated with the handle and releases its resources.

-   **Communication Functions (Bridging to `DriverXPCManager`):**
    -   `void FWADriverXPCBridge_GetSharedMemoryName(FWADriverXPCBridgeHandle handle, void (*callback)(const char* shm_name, void* context), void* context);`:
        -   Initiates an asynchronous request to get the shared memory name from the daemon via XPC.
        -   The `handle` identifies the bridge instance.
        -   `callback`: A C function pointer that will be called when the shared memory name is retrieved. It receives the name as a C string (`const char*`) and the `context` pointer provided by the caller.
        -   `context`: A `void*` for user-defined data to be passed to the callback.
    -   `void FWADriverXPCBridge_NotifyDriverPresence(FWADriverXPCBridgeHandle handle, bool isPresent);`:
        -   Notifies the daemon (via `DriverXPCManager`) whether the driver is present (loaded/active) or not.
    -   `void FWADriverXPCBridge_ForwardLogMessage(FWADriverXPCBridgeHandle handle, int level, const char* message);`:
        -   Forwards a log message from the C++ driver code to the daemon via XPC.
    -   *(Other functions like `SendControlCommand` and `NotifyDeviceStatusChanged` would likely also be declared here if they are to be exposed through this C bridge, following a similar pattern.)*

**Overall Role:**
The `FWADriverXPCBridge.h` provides a clean C Application Programming Interface (C-API) that abstracts the Objective-C++ implementation of XPC communication. This allows the core C++ logic of the driver to remain independent of Objective-C details, promoting modularity and potentially making it easier to manage the interface between different language components within the driver. The C++ code calls these C functions, which internally delegate to the methods of an `FWADriverXPCBridgeImpl` Objective-C++ object, which in turn uses the `DriverXPCManager`.
