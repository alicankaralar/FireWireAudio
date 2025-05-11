# Summary for src/driver/Info.plist

This file is the property list (`Info.plist`) for the `com_FWAudio_driver` kernel extension (kext). It provides essential metadata that the macOS kernel and IOKit use to manage the kext.

**Key Information Defined:**

-   **`CFBundleIdentifier`**: `net.mrmidi.FWDriver`
    -   The unique bundle identifier for this kernel extension. This is how the system uniquely identifies the kext.

-   **`CFBundleName`**: `FWADriver`
    -   The human-readable name of the kext.

-   **`CFBundleVersion` / `CFBundleShortVersionString`**: `1.0.0` / `1.0`
    -   Version information for the kext.

-   **`IOKitPersonalities`**:
    -   This dictionary contains one or more "personalities" for the driver. Each personality defines a set of matching criteria that IOKit uses to determine if this driver should be loaded for a particular piece of hardware.
    -   The primary personality here is named `FWADriverPersonalities` (matching the C-array name in `FWADriverInit.cpp`).
    -   **`IOClass`**: `com_FWAudio_driver` (The main C++ class of the kext that IOKit will instantiate).
    -   **`IOProviderClass`**: `IOFireWireUnit` (Specifies that this driver attaches to instances of `IOFireWireUnit`).
    -   **`IOProbeScore`**: `20000` (A score used by IOKit to resolve conflicts if multiple drivers match the same hardware).
    -   **`CFBundleIdentifier`**: `net.mrmidi.FWDriver` (Repeated here for the personality).
    -   **`IOUserClientClass`**: `FWADriverUserClient` (The name of the user client class that user-space applications will use to communicate with this driver instance).
    -   **`IOPropertyMatch`**:
        -   Contains a dictionary that specifies a property match on a child nub. It requires an `IOFireWireAVCUnit` child with the property `AVCCommandSetSpecIDC` equal to `0xA00020` (decimal 655392). This ensures the driver only loads for FireWire devices that have a specific type of AV/C audio subunit.

-   **`OSBundleLibraries`**:
    -   A dictionary specifying the kernel extensions that this kext depends on. The system ensures these dependencies are met before loading this kext.
    -   Dependencies include:
        -   `com.apple.iokit.IOFireWireFamily` (for FireWire support)
        -   `com.apple.iokit.IOAudioFamily` (for Core Audio integration)
        -   Various KPI (Kernel Programming Interface) libraries like `com.apple.kpi.bsd`, `com.apple.kpi.iokit`, `com.apple.kpi.libkern`, `com.apple.kpi.mach`.

-   **`OSBundleRequired`**: `Root` (Indicates the kext is loaded into the kernel address space).

**Overall Role:**
The `Info.plist` is a critical metadata file for any kext. For `FWADriver`, it:
1.  Identifies the kext to the system.
2.  Defines the specific hardware characteristics (via `IOKitPersonalities`) that will cause this kext to be loaded.
3.  Specifies the main C++ class (`IOClass`) that IOKit should instantiate.
4.  Declares the user client class (`IOUserClientClass`) for user-space communication.
5.  Lists its dependencies on other kernel components (`OSBundleLibraries`).
