# Summary for src/Isoch/CMakeLists.txt

This `CMakeLists.txt` file is responsible for building a static library named `FWA_Isoch`. This library encapsulates the core logic for handling isochronous (real-time, time-sensitive) data streams over FireWire, which is essential for audio transmission. It includes components for AMDT-P (Audio and Music Data Transmission Protocol) packetization and depacketization, buffer management, and interaction with IOKit's isochronous streaming APIs.

**Key Components and Configuration:**

-   **Library Target (`FWA_Isoch`):**
    -   `add_library(FWA_Isoch STATIC ...)`: Defines a static library named `FWA_Isoch`.
    -   **Source Files:** Lists all the C++ source files that constitute this library. These cover various aspects of isochronous streaming:
        -   **Core AMDT-P Handling:**
            -   `core/AmdtpReceiver.cpp`: Implements logic to receive and depacketize AMDT-P packets.
            -   `core/AmdtpTransmitter.cpp`: Implements logic to packetize audio data into AMDT-P format and transmit it.
            -   `core/AmdtpTransmitStreamProcessor.cpp`: Likely manages the processing pipeline for outgoing AMDT-P streams.
        -   **Stream Management:**
            -   `AudioDeviceStream.cpp`: Manages a single audio stream (input or output) for a device.
            -   `IsoStreamHandler.cpp`: A higher-level handler for managing multiple isochronous streams.
        -   **Buffer Management:**
            -   `core/IsochBufferManager.cpp`: General isochronous buffer management.
            -   `core/IsochDoubleBufferManager.cpp`: Implements double buffering for stream data.
            -   `core/IsochTransmitBufferManager.cpp`: Specific buffer management for transmit streams.
        -   **DCL (Data Command List) Management for IOKit:**
            -   `core/IsochDCLManager.cpp`: Manages DCLs for receiving isochronous data with IOKit.
            -   `core/IsochTransmitDCLManager.cpp`: Manages DCLs for transmitting isochronous data.
        -   **Packet Processing and Provision:**
            -   `core/IsochPacketProcessor.cpp`: Processes incoming/outgoing isochronous packets.
            -   `core/IsochPacketProvider.cpp`: Provides packets to the transmitter.
        -   **Clocking and Synchronization:**
            -   `core/AudioClockPLL.cpp`: Implements a Phase-Locked Loop for audio clock synchronization.
        -   **Utility Components:**
            -   `utils/AmdtpHelpers.cpp`: Helper functions for AMDT-P.
            -   `utils/CIPHeaderHandler.cpp`: Handles CIP (Common Isochronous Packet) headers.
            -   `utils/RunLoopHelper.cpp`: Utilities for integrating with `CFRunLoop`.
        -   **Shared/Higher-Level Management:**
            -   `SharedManagers.cpp`: Possibly provides access to shared manager instances within the Isoch module.
            -   `components/AmdtpTransportManager.cpp`: Manages AMDT-P transport sessions.
            -   `components/TransmitterComponents.cpp`: Groups components related to transmission.
        -   (Other files like `IsochManager.cpp`, `IsochMonitoringManager.cpp`, `IsochPortChannelManager.cpp`, `ReceiverFactory.cpp` contribute to the overall isochronous framework.)

-   **Include Directories (`target_include_directories`):**
    -   `PUBLIC ${CMAKE_SOURCE_DIR}/include`: Makes the main project's `include` directory (containing public headers for `FWA`, `Isoch`, `shared`, etc.) available to this library and any targets linking against it.
    -   `PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}`: Makes the current directory (`src/Isoch/`) available for internal includes.

-   **Linked Libraries (`target_link_libraries`):**
    -   `PUBLIC FWA`: Links against the main `FWA` library. This is a crucial dependency, as the `FWA_Isoch` library will need to interact with `AudioDevice` objects (to get plug information, send configuration commands via `CommandInterface`, etc.).
    -   `PUBLIC spdlog::spdlog`: For logging.
    -   `PUBLIC "-framework IOKit"`: For IOKit APIs, especially those related to FireWire and isochronous streaming.
    -   `PUBLIC "-framework CoreFoundation"` and `PUBLIC "-framework Foundation"`: For CoreFoundation and Foundation framework utilities.

-   **Compiler Options (`target_compile_options`):**
    -   `-std=c++23`: Sets the C++ language standard.
    -   `-stdlib=libc++`: Specifies the libc++ standard library.

**Overall Role:**
The `FWA_Isoch` library is a specialized module focused on the real-time aspects of FireWire audio communication. It handles the complexities of:
1.  Setting up and managing isochronous data channels on the FireWire bus.
2.  Implementing the AMDT-P protocol for packetizing audio data for transmission and depacketizing received data.
3.  Managing data buffers and synchronization for smooth audio playback and recording.
4.  Interacting with IOKit's low-level isochronous streaming APIs.
It works closely with the `FWA` library, which provides the device abstraction and control, to enable actual audio I/O.
