# Summary for src/Isoch/core/AmdtpReceiver.cpp

This C++ file implements the `FWA::Isoch::AmdtpReceiver` class. This class is responsible for receiving and processing AMDT-P (Audio and Music Data Transmission Protocol) packets arriving on a FireWire isochronous stream. It depacketizes these packets to extract the underlying audio or MIDI data.

**Key Functionalities:**

-   **Constructor (`AmdtpReceiver::AmdtpReceiver`):**
    -   Takes parameters such as the expected `AudioStreamFormat`, a data callback function pointer (to deliver processed audio data), user context for the callback, and an `spdlog::logger`.
    -   Initializes member variables, including the expected format, callback pointers, and logger.

-   **Setup and Configuration (`setup`):**
    -   `bool setup(IOFireWireLibIsochPortRef isochPort, uint8_t channel, uint32_t packetSize, uint32_t numBuffers, uint32_t numFramesPerBuffer)`:
        -   Configures the receiver for a specific isochronous channel and port.
        -   **DCL Management:** Initializes an `IsochDCLManager` (`_dclManager`) to manage IOKit Data Command Lists for receiving data. DCLs describe the memory buffers where incoming isochronous packets will be placed by the hardware.
        -   **Buffer Management:** Initializes an `IsochBufferManager` (`_bufferManager`) to allocate and manage these receive buffers.
        -   **IOKit Receive Stream:** Creates and configures an `IOFireWireReceiveStreamRef` (IOKit object for receiving isochronous data) for the specified `isochPort` and `channel`.
        -   Sets up completion routines for the DCLs. These routines are called by IOKit when a DCL (and its associated buffer) has been filled with data.
        -   Prepares and queues the initial set of DCLs to start receiving data.

-   **Stream Control (`start`, `stop`):**
    -   `bool start()`:
        -   Starts the IOKit isochronous receive stream operation (e.g., by calling `(*_receiveStream)->Start(_receiveStream)`).
        -   Sets an internal state flag to indicate the stream is running.
    -   `bool stop()`:
        -   Stops the IOKit isochronous receive stream.
        -   Aborts any pending DCLs.
        -   Releases IOKit resources (DCLs, buffers, stream object).
        -   Sets the internal state flag to stopped.

-   **DCL Completion Handling (Callback from IOKit):**
    -   A static C-style callback function (e.g., `AmdtpReceiver::dclCompletionCallback`) is registered with IOKit for DCL completions.
    -   When IOKit fills a buffer, this callback is invoked.
    -   The static callback retrieves the `AmdtpReceiver` instance (passed as `refCon`) and calls an instance method (e.g., `handleDCLCompletion`).
    -   `void handleDCLCompletion(FWIsochDCL* dcl, IOReturn status)`:
        -   This instance method processes the completed DCL.
        -   It iterates through the received isochronous packets within the DCL's buffer.
        -   For each packet:
            -   **CIP Header Parsing:** Parses the CIP (Common Isochronous Packet) header using `CIPHeaderHandler` to extract fields like Data Block Size (DBS), Data Block Count (DBC), Format Dependent Field (FDF), and SYT (timestamp).
            -   **AMDT-P Validation:** Validates the AMDT-P packet structure (e.g., checks FDF for audio/MIDI, presence of SYT).
            -   **Data Extraction:** Extracts the actual audio/MIDI payload from the data blocks within the AMDT-P packet.
            -   **Timestamp Handling:** Processes the SYT value to maintain timing information.
            -   **Data Delivery:** If a data callback (`_dataCallback`) was registered, it's invoked with the extracted audio data, its format, timestamp, and frame count.
        -   Re-queues the DCL (with its buffer) to IOKit to continue receiving more data.

-   **Error Handling and Synchronization:**
    -   Detects and handles packet loss or corruption (e.g., based on DBC sequence numbers, CRC errors if checked).
    -   Manages resynchronization if SYT timestamps indicate discontinuities or stream restarts.
    -   Logs errors and status information.

**Overall Role:**
The `AmdtpReceiver` is a critical component for receiving audio or MIDI data from a FireWire device. It interfaces with IOKit's low-level isochronous receive mechanisms, manages data buffers and DCLs, and performs the complex task of parsing incoming AMDT-P packets according to the IEC 61883-6 specification. Once the raw audio/MIDI data is extracted and timestamped, it's made available to the rest of the FWA library or application, typically via a callback mechanism.
