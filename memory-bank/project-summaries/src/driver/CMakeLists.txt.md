# Summary for src/driver/CMakeLists.txt

This CMakeLists.txt file defines a macOS kernel extension (kext) target named `com_FWAudio_driver`. 

Key aspects:
- **Target Definition:** Creates a `MODULE` library (typical for kexts) named `com_FWAudio_driver`.
- **Source Files:** Includes `FWADriver.cpp`, `FWADriverInit.cpp`, `FWADriverDevice.cpp`, `FWADriverHandler.cpp`, `DriverXPCManager.mm`, and `FWADriverXPCBridge.mm`.
- **Bundle Properties:** Sets `FRAMEWORK_VERSION` (acting as CFBundleVersion), `MACOSX_BUNDLE_GUI_IDENTIFIER` (acting as CFBundleIdentifier `net.mrmidi.FWDriver`), `MACOSX_BUNDLE_BUNDLE_NAME`, and `MACOSX_BUNDLE_SHORT_VERSION_STRING`.
- **Compiler Flags:**
    - Sets C++ standard to 17 (`-std=c++17`).
    - Sets macOS deployment target to 10.15 (`-mmacosx-version-min=10.15`).
    - Includes various warning flags (`-Wall`, `-Wextra`, etc.).
    - Defines `DEBUG=1` and `LOGGING_LEVEL_DEBUG=1` for debug builds.
    - Adds `-fno-exceptions`, `-fno-rtti` (common for kernel code).
- **Linker Flags:**
    - Links against the Kernel framework (`-framework Kernel`).
    - Specifies `-nostdlib` and other kernel-specific linker options.
- **Include Directories:** Adds `../../../include` and the current source directory (`.`) to the include path.
- **Output Directory:** Sets the library output path to `${CMAKE_BINARY_DIR}/driver`.
- **Xcode Version:** Sets `XCODE_VERSION` to 80, which might be relevant for compatibility or specific Xcode features used in the build process.

This file is crucial for building the kernel-side driver component of the FireWire audio system.
