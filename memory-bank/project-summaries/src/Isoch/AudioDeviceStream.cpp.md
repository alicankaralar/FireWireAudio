# Summary for src/Isoch/AudioDeviceStream.cpp

This C++ file implements the `FWA::Isoch::AudioDeviceStream` class. This class is responsible for managing a single isochronous audio stream (either input or output) for a FireWire audio device. It handles the setup, starting, stopping, and data flow for that stream.

**Key Functionalities:**

-   **Constructor (`AudioDeviceStream::AudioDeviceStream`):**
    -   Takes parameters such as:
        -   A reference or shared pointer to the parent `FWA::AudioDevice` object.
        -   `StreamDirection` (Input or Output).
        -   The plug ID on the device this stream is associated with.
        -   Desired `AudioStreamFormat` (sample rate, bit depth, channels).
        -   An `spdlog::logger`.
    -   Initializes member variables, including storing the device reference, direction, plug ID, and desired format.

-   **Stream Setup (`setupStream`):**
    -   `bool setupStream()`:
        -   Validates if the desired format is supported by the device's plug (by querying the `AudioDevice` and its `AudioPlug` objects).
        -   **Isochronous Channel Allocation:** Interacts with the FireWire bus (via IOKit, likely through the `IOFireWireLibDeviceRef` held by `AudioDevice`) to allocate an isochronous channel and bandwidth.
        -   **AMDT-P Packetizer/Depacketizer Setup:**
            -   If it's an **output stream**, it would initialize an `AmdtpTransmitter` (or a component that uses it, like `AmdtpTransmitStreamProcessor`). This component is responsible for taking audio data, packetizing it according to the AMDT-P (Audio and Music Data Transmission Protocol) specification, and preparing it for transmission over the allocated isochronous channel.
            -   If it's an **input stream**, it would initialize an `AmdtpReceiver`. This component receives AMDT-P packets from the isochronous channel, depacketizes them, and extracts the raw audio data.
        -   **Buffer Management:** Sets up internal buffers for audio data (e.g., a ring buffer to decouple the application/driver side from the isochronous transmission timing). For output, this buffer would be filled by the audio source (e.g., `ShmIsochBridge`). For input, this buffer would be filled by the `AmdtpReceiver` and read by the audio sink.
        -   Sends AV/C commands to the device (via `AudioDevice`'s `CommandInterface`) to configure the device to start or stop sending/receiving on the specific plug and isochronous channel, and to set the stream format on the device side.

-   **Stream Control (`start`, `stop`):**
    -   `bool start()`:
        -   Performs final checks.
        -   Commands the device (via AV/C) to start streaming on the configured plug.
        -   Starts the IOKit isochronous stream operation (e.g., using `IOFireWireIsochStreamRef`).
        -   Starts the internal data processing threads (e.g., for the `AmdtpTransmitter` or `AmdtpReceiver`).
        -   Updates the stream state to "running".
    -   `bool stop()`:
        -   Stops the IOKit isochronous stream operation.
        -   Commands the device (via AV/C) to stop streaming on the plug.
        -   Stops internal data processing threads.
        -   Releases allocated isochronous channel and bandwidth.
        -   Updates the stream state to "stopped".

-   **Data Handling:**
    -   **For Output Streams:**
        -   `size_t writeData(const void* buffer, size_t numBytes)`: A method called by the audio source to provide audio data to be sent. This data is typically placed into an internal buffer, from which the `AmdtpTransmitter` will pull it.
    -   **For Input Streams:**
        -   `size_t readData(void* buffer, size_t maxBytes)`: A method called by the audio sink to retrieve received audio data. This data comes from an internal buffer filled by the `AmdtpReceiver`.
        -   Alternatively, it might use a callback mechanism to notify a listener when new input data is available.

-   **State Management:**
    -   Tracks the current state of the stream (e.g., `kStreamStateStopped`, `kStreamStateStarting`, `kStreamStateRunning`, `kStreamStateStopping`, `kStreamStateError`).
    -   Provides methods to query the current state and format.

-   **Error Handling:**
    -   Uses `std::expected` or returns boolean success/failure for its operations.
    -   Logs errors using the provided logger.

**Overall Role:**
The `AudioDeviceStream` class is a critical component for managing the actual audio data flow to and from a FireWire device. It bridges the gap between the higher-level representation of an audio device and its streams, and the low-level details of FireWire isochronous communication and the AMDT-P protocol. It handles resource allocation on the FireWire bus, configures the hardware device for streaming, and manages the packetization/depacketization of audio data. There would typically be one `AudioDeviceStream` instance for each active input or output audio path.
