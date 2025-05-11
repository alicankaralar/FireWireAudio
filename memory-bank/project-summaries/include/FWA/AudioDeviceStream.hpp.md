# Summary for include/FWA/AudioDeviceStream.hpp

This C++ header file defines the `FWA::Isoch::AudioDeviceStream` class. This class is designed to represent and manage a single isochronous audio stream, which can be either an input stream (recording from the device) or an output stream (playback to the device), associated with a specific FireWire audio device.

**Key Declarations and Components:**

-   **Namespace `FWA::Isoch`:** The class is part of the `Isoch` sub-namespace, indicating its role in isochronous stream handling.

-   **Includes:**
    -   `<IOKit/firewire/IOFireWireLib.h>`: For IOKit FireWire types like `IOFireWireLibDeviceRef`.
    -   `<expected>`: For `std::expected` error handling.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `../Enums.hpp`: For `StreamDirection` enum.
    -   `../Error.h`: For `IOKitError` enum and error handling utilities.
    -   `AudioStreamFormat.hpp`: For the `AudioStreamFormat` struct.
    -   (Potentially other internal headers for AMDT-P handlers, DCL managers, etc., would be included in the `.cpp` file).

-   **Class `AudioDeviceStream`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `AudioDeviceStream(IOFireWireLibDeviceRef deviceRef, StreamDirection direction, uint8_t plugID, const AudioStreamFormat& format, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the stream with the IOKit device reference, stream direction (input/output), the device plug ID it's associated with, the desired audio format, and a logger.
        -   **Destructor:**
            -   `~AudioDeviceStream();`
            -   Responsible for ensuring the stream is stopped and resources are released.
        -   **Core Stream Management Methods:**
            -   `std::expected<void, IOKitError> setupStream();`:
                -   Performs the necessary setup to configure the stream. This includes validating the format, allocating FireWire isochronous channel and bandwidth, setting up AMDT-P packetizers/depacketizers (`AmdtpTransmitter`/`AmdtpReceiver`), preparing IOKit DCLs and buffers, and sending AV/C commands to the device to prepare it for streaming on the specified plug.
            -   `std::expected<void, IOKitError> start();`:
                -   Starts the actual isochronous data flow on the FireWire bus and begins internal data processing.
            -   `std::expected<void, IOKitError> stop();`:
                -   Stops the isochronous data flow, tells the device to stop streaming on the plug, and releases allocated FireWire bus resources and IOKit objects.
        -   **State and Information Accessors:**
            -   `bool isRunning() const;`: Returns `true` if the stream is currently active.
            -   `StreamDirection getDirection() const;`: Returns whether it's an input or output stream.
            -   `uint8_t getPlugID() const;`: Returns the associated device plug ID.
            -   `const AudioStreamFormat& getFormat() const;`: Returns the current audio format of the stream.
            -   `uint32_t getIsochChannel() const;`: Returns the allocated FireWire isochronous channel number (valid after successful setup).
        -   **Data Handling Interface (Conceptual - details in .cpp):**
            -   For output streams: A method like `writeData(const void* buffer, size_t numBytes)` to push audio data into the stream.
            -   For input streams: A method like `readData(void* buffer, size_t maxBytes)` to pull audio data, or a callback mechanism to deliver received data.

    -   **Private Members (Conceptual - implementation details in .cpp):**
        -   `_deviceRef`: `IOFireWireLibDeviceRef` (handle to the IOKit device object).
        -   `_direction`: `StreamDirection`.
        -   `_plugID`: `uint8_t`.
        -   `_format`: `AudioStreamFormat`.
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.
        -   `_isRunning`: `atomic<bool>` or similar to track the running state.
        -   `_isochChannel`: `uint32_t` (the allocated FireWire isochronous channel).
        -   `_ioKitStreamRef`: An IOKit stream object (e.g., `IOFireWireIsochStreamRef`, `IOFireWireReceiveStreamRef`, or `IOFireWireTransmitStreamRef`).
        -   `_amdtpHandler`: A pointer or shared pointer to an `AmdtpTransmitter` or `AmdtpReceiver`.
        -   Pointers or shared pointers to DCL managers (`IsochDCLManager`, `IsochTransmitDCLManager`) and buffer managers (`IsochBufferManager`, `IsochTransmitBufferManager`).
        -   Internal buffers or queues for audio data.

**Overall Role:**
The `AudioDeviceStream` class is a fundamental component for managing real-time audio data transfer with a FireWire device. It encapsulates the logic for:
1.  Interacting with IOKit to allocate and manage FireWire isochronous resources (channels, bandwidth).
2.  Setting up and controlling the IOKit isochronous stream objects.
3.  Coordinating with AMDT-P handlers (`AmdtpTransmitter` for output, `AmdtpReceiver` for input) to ensure correct packetization and depacketization of audio data according to the IEC 61883-6 standard.
4.  Managing data buffers and the flow of audio data between the application/driver side and the FireWire bus.
An instance of `AudioDeviceStream` would be created by `IsoStreamHandler` for each active input or output audio path.
