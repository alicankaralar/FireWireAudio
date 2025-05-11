# Summary for src/driver/FWADriverInit.hpp

This header file declares the C-style global entry and exit point functions for the `com_FWAudio_driver` kernel extension (kext). These functions are required by the macOS kernel to load and unload kexts.

**Key Declarations:**

-   **`#include <IOKit/IOService.h>`:** Includes a fundamental IOKit header, likely for types such as `kmod_info_t` used in the function signatures.

-   **`extern "C"` block:** Ensures that the declared functions have C linkage, making them callable directly by the kernel's kext loading mechanism, which expects C function signatures.

-   **`kern_return_t FWADriver_start(kmod_info_t* ki, void* d);`**:
    -   Declaration of the kext's main start function.
    -   This function is called by the kernel when the kext is successfully matched to hardware (based on its personality dictionary) and is being loaded.
    -   It's responsible for any global initialization the kext needs, such as setting up communication with other system components (like the user-space daemon via XPC in this project).
    -   The implementation is found in `FWADriverInit.cpp`.

-   **`kern_return_t FWADriver_stop(kmod_info_t* ki, void* d);`**:
    -   Declaration of the kext's main stop function.
    -   This function is called by the kernel when the kext is about to be unloaded.
    -   It's responsible for cleaning up any global resources, terminating connections, and ensuring the kext can be safely removed from the system.
    -   The implementation is found in `FWADriverInit.cpp`.

**Overall Role:**
This header file provides the forward declarations for the kext's primary lifecycle functions. These functions bridge the kernel's kext management system with the C++ code of the driver.
