# Summary for src/FWA/CMakeLists.txt

This `CMakeLists.txt` file is responsible for building the core user-space FireWire Audio library, named `FWA`. It defines this library as a static library and specifies its source files, include directories, and dependencies.

**Key Components and Configuration:**

-   **Library Target (`FWA`):**
    -   `add_library(FWA STATIC ...)`: Defines a static library named `FWA`.
    -   **Source Files:** Lists all the C++ source files that constitute the library. This includes:
        -   Core device representation: `AudioDevice.cpp`, `DeviceInfo.cpp`
        -   AV/C command handling: `CommandInterface.cpp`
        -   Descriptor parsing: `DescriptorReader.cpp`, `DescriptorAccessor.cpp`, `DescriptorSpecifier.cpp`, `DescriptorUtils.cpp`, `DeviceParser.cpp`, `MusicSubunitDescriptorParser.cpp`, `PlugDetailParser.cpp`, `AVCInfoBlock.cpp`
        -   Subunit management: `AudioSubunit.hpp` (likely a typo, should be `.cpp`), `MusicSubunit.cpp`, `MusicSubunitCapabilities.cpp`, `SubunitDiscoverer.cpp`
        -   Plug management: `AudioPlug.cpp`, `UnitPlugDiscoverer.cpp`
        -   Stream formats: `AudioStreamFormat.cpp`
        -   DICE-specific implementations: `dice/DiceAudioDevice.cpp`, `dice/DiceEAP.cpp`, `dice/DiceRouter.cpp`
        -   Device discovery: `IOKitFireWireDeviceDiscovery.cpp`
        -   Helper utilities: `Helpers.cpp`, `JsonHelpers.cpp`
        -   (Note: `Subunit.hpp` is listed, which is unusual for a source file list; typically it would be `Subunit.cpp` if it has an implementation, or it's just a header-only class included elsewhere.)

-   **Include Directories (`target_include_directories`):**
    -   `PUBLIC ${CMAKE_SOURCE_DIR}/include`: Makes the main project's `include` directory available to both the `FWA` library itself and any code that links against `FWA`. This is where public headers for the FWA library and its dependencies would reside.
    -   `PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}`: Makes the current directory (`src/FWA/`) available for includes only within the `FWA` library itself (for internal headers).

-   **Linked Libraries (`target_link_libraries`):**
    -   Specifies `PUBLIC` linkage, meaning that targets linking against `FWA` will also link against these dependencies.
    -   `spdlog::spdlog`: For structured logging.
    -   `FWA_XPC`: Links against the `FWA_XPC` static library (defined in `src/FWA/XPC/CMakeLists.txt`), which handles XPC communication. This implies the FWA library uses XPC, possibly to communicate with a daemon or other services.
    -   `nlohmann_json::nlohmann_json`: For JSON parsing and serialization.
    -   macOS Frameworks:
        -   `"-framework IOKit"`: Essential for interacting with hardware and I/O services, including FireWire.
        -   `"-framework CoreFoundation"`: Core Foundation framework.
        -   `"-framework Foundation"`: Foundation framework, providing base Objective-C classes and utilities.

-   **Compiler Options (`target_compile_options`):**
    -   `-std=c++23`: Sets the C++ language standard to C++23.
    -   `-stdlib=libc++`: Specifies the use of the libc++ standard library (common on macOS).

**Overall Role:**
This `CMakeLists.txt` builds `libFWA.a`, the static library that contains the primary user-space logic for interacting with FireWire audio devices. It handles device discovery, parsing device capabilities from descriptors, sending AV/C commands, and provides specialized support for DICE chipsets. Its linkage to `FWA_XPC` indicates its involvement in inter-process communication.
