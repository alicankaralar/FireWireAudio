# Summary for src/FWA/XPC/CMakeLists.txt

This `CMakeLists.txt` file is responsible for building a static library named `FWA_XPC`. This library encapsulates the client-side XPC (Cross-Process Communication) logic that the main `FWA` library uses, presumably to communicate with the `FWADaemon` XPC service.

**Key Components and Configuration:**

-   **Library Target (`FWA_XPC`):**
    -   `add_library(FWA_XPC STATIC ...)`: Defines a static library named `FWA_XPC`.
    -   **Source Files:**
        -   `XPCBridge.mm`: Implements the `XPCBridge` class, which acts as the primary client-side interface for connecting to and interacting with the `FWADaemon` XPC service.
        -   `XPCReceiverClient.mm`: Implements the `XPCReceiverClient` class, which likely handles receiving asynchronous notifications or callbacks from the `FWADaemon` over XPC.
        -   `MixedAudioBuffer.m`: Implements the `MixedAudioBuffer` class, a data structure likely used for packaging audio data to be sent over XPC (e.g., from a client application to the daemon, or vice-versa if the daemon also sends audio back).

-   **Include Directories (`target_include_directories`):**
    -   `PUBLIC ${CMAKE_SOURCE_DIR}/include`: Makes the main project's `include` directory available to both the `FWA_XPC` library itself and any code that links against `FWA_XPC` (which would be the `FWA` library). This is crucial for accessing shared XPC protocol definitions (like `FWADaemonControlProtocol.h` and `FWAClientNotificationProtocol.h`) and other public headers.

-   **Linked Libraries (`target_link_libraries`):**
    -   Specifies `PUBLIC` linkage, meaning that targets linking against `FWA_XPC` (i.e., `FWA`) will also link against these dependencies.
    -   `spdlog::spdlog`: For structured logging.
    -   `nlohmann_json::nlohmann_json`: For JSON parsing and serialization, if XPC messages involve JSON payloads.
    -   macOS Frameworks:
        -   `"-framework Foundation"`: Essential for Objective-C runtime, `NSXPCConnection`, and other fundamental classes used in XPC.
        -   `"-framework CoreFoundation"`: Core Foundation framework.

-   **Compiler Options (`target_compile_options`):**
    -   `-std=c++23`: Sets the C++ language standard to C++23.
    -   `-stdlib=libc++`: Specifies the use of the libc++ standard library.

**Overall Role:**
This `CMakeLists.txt` builds `libFWA_XPC.a`, a static library that provides the XPC client capabilities for the main `FWA` library. The `XPCBridge` class within this library will allow the `FWA` library (e.g., `AudioDevice` or `DeviceController`) to establish a connection to the `FWADaemon` service, send requests (like control commands or status queries), and the `XPCReceiverClient` will handle incoming messages or notifications from the daemon. This enables a separation of concerns, keeping the XPC communication details encapsulated within this dedicated library.
