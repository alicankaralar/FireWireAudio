# Summary for src/Isoch/IsoStreamHandler.cpp

This C++ file implements the `FWA::Isoch::IsoStreamHandler` class. This class is a high-level manager responsible for creating, configuring, starting, stopping, and managing all active isochronous audio streams (both input and output) associated with a single `FWA::AudioDevice` instance.

**Key Functionalities:**

-   **Constructor (`IsoStreamHandler::IsoStreamHandler`):**
    -   Takes a `std::shared_ptr<FWA::AudioDevice>` representing the device these streams belong to, and an `std::shared_ptr<spdlog::logger>`.
    -   Stores these dependencies.

-   **Stream Creation and Management:**
    -   `std::shared_ptr<AudioDeviceStream> createOutputStream(uint8_t plugID, const AudioStreamFormat& format, IsochStreamCallback callback = nullptr)`:
        -   Creates, configures, and potentially starts an output `AudioDeviceStream`.
        -   Validates that the `plugID` and `format` are supported by the `_audioDevice`.
        -   Instantiates an `AudioDeviceStream` object for output.
        -   Calls `setupStream()` on the new stream object, which involves:
            -   Allocating FireWire isochronous channel and bandwidth.
            -   Setting up AMDT-P packetization (`AmdtpTransmitter`).
            -   Configuring the hardware device via AV/C commands to expect data on this stream.
        -   If successful, stores the stream in a collection (e.g., `_outputStreams`).
        -   Optionally takes a callback for stream events (e.g., underrun, error).
    -   `std::shared_ptr<AudioDeviceStream> createInputStream(uint8_t plugID, const AudioStreamFormat& format, IsochStreamCallback callback = nullptr)`:
        -   Similar to `createOutputStream`, but for input streams.
        -   Sets up an `AmdtpReceiver` for depacketizing incoming AMDT-P data.
        -   Stores the stream in `_inputStreams`.
    -   `destroyStream(std::shared_ptr<AudioDeviceStream> stream)`:
        -   Stops the given stream by calling its `stop()` method (which releases isochronous resources and tells the hardware to stop).
        -   Removes it from the internal collections.

-   **Global Stream Control:**
    -   `startAllStreams()`: Iterates through all managed streams and calls `start()` on each.
    -   `stopAllStreams()`: Iterates and calls `stop()` on each.

-   **Data Path Connection (Conceptual Integration):**
    -   **Output Path:** For output streams, the `IsoStreamHandler` (or the `AudioDeviceStream` it creates) needs to be connected to a data source. In this system, this source is the `ShmIsochBridge`, which reads audio data from the shared memory ring buffer filled by the kernel driver. The `AudioDeviceStream`'s `writeData` method would be called by `ShmIsochBridge`.
    -   **Input Path:** For input streams, the `AudioDeviceStream` (specifically its `AmdtpReceiver`) produces audio data. This data needs to be delivered to a sink. This could be:
        -   Another shared memory ring buffer for a client application to read.
        -   A callback mechanism to a registered data consumer.
        -   Directly to an XPC client if the architecture supports it.

-   **State Management and Querying:**
    -   Maintains lists or maps of active `_outputStreams` and `_inputStreams`.
    -   Provides methods to get a list of active streams or query the status of a specific stream.

-   **Resource Coordination:**
    -   Ensures that isochronous channel numbers and bandwidth allocation are managed correctly across multiple streams for the same device, preventing conflicts.

-   **Error Handling and Logging:**
    -   Uses the provided logger to report errors during stream setup, start, stop, or data handling.
    -   Methods return `std::shared_ptr` (which can be null on failure) or `std::expected` to indicate success/failure.

**Overall Role:**
The `IsoStreamHandler` acts as the primary orchestrator for all isochronous audio I/O related to a specific `AudioDevice`. It abstracts the complexities of managing multiple `AudioDeviceStream` instances, ensuring they are correctly configured based on device capabilities and user requests. It forms a bridge between the logical `AudioDevice` representation and the concrete `AudioDeviceStream` objects that handle the real-time data transfer over FireWire. It's a central piece for enabling audio playback and recording functionality.
