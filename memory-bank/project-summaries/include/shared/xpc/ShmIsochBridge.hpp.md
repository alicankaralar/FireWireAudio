# Summary for include/shared/xpc/ShmIsochBridge.hpp

This C++ header file defines the `ShmIsochBridge` class. This class is designed as a singleton and acts as a crucial bridge component within the `FWADaemon`. Its primary responsibility is to take audio data chunks, which have been read from the POSIX shared memory ring buffer (populated by the kernel driver and consumed by `RingBufferManager`), and forward this data to the appropriate isochronous transmission pipeline for output to the FireWire audio device.

**Key Declarations and Components:**

-   **Includes:**
    -   `"../SharedMemoryStructures.hpp"`: For the `RTShmRing::AudioChunk_POD` structure, which defines the format of audio data chunks in shared memory.
    -   `../../Isoch/interfaces/ITransmitPacketProvider.hpp"`: For the `FWA::Isoch::ITransmitPacketProvider` interface. The `ShmIsochBridge` will push audio data to an object implementing this interface (likely an `IsochPacketProvider` linked to an `AmdtpTransmitStreamProcessor`).
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<atomic>`: For `std::atomic` used in the internal lock-free queue.
    -   `<thread>`: For `std::thread` to run the worker loop.
    -   `<vector>`: For `std::vector<uint8_t>` to store audio data in the internal queue.
    -   `<array>`: For `std::array` to implement the fixed-size internal queue.
    -   `<mutex>`: (Potentially for auxiliary state, though the main queue is lock-free).
    -   `<memory>`: For `std::shared_ptr`.

-   **Class `ShmIsochBridge`:**
    -   **Singleton Access:**
        -   `static ShmIsochBridge& instance();`: Provides global access to the single instance of `ShmIsochBridge`.
    -   **Constructor and Destructor (Private/Deleted):**
        -   `ShmIsochBridge();`
        -   `~ShmIsochBridge();`
        -   Copy and move constructors/assignment operators are deleted to enforce the singleton pattern.

    -   **Internal Queue Implementation:**
        -   `struct QueueItem { std::vector<uint8_t> data; };`: A simple struct to hold a chunk of audio data within the internal queue.
        -   `static constexpr size_t kQueueCapacity = 256;`: Defines the capacity of the internal SPSC queue (should be a power of 2 for efficient modulo).
        -   `std::array<QueueItem, kQueueCapacity> _queue;`: The fixed-size array acting as the circular buffer.
        -   `std::atomic<size_t> _writeIndex{0};`: Atomic counter for the next write position.
        -   `std::atomic<size_t> _readIndex{0};`: Atomic counter for the next read position.

    -   **Public Interface:**
        -   `void start(FWA::Isoch::ITransmitPacketProvider* provider);`:
            -   Stores the provided `provider` (which is the entry point to the isochronous transmit pipeline for a specific stream).
            -   Sets an internal `_running` flag to `true`.
            -   Creates and starts the `_workerThread`, which executes the private `workerLoop()` method.
        -   `void stop();`:
            -   Sets `_running` to `false` to signal the worker thread to terminate.
            -   Joins the `_workerThread` to wait for its completion.
        -   `bool enqueue(const RTShmRing::AudioChunk_POD& chunk);`:
            -   This method is called by the `RingBufferManager` (after it pops a chunk from the shared memory).
            -   It attempts to push the audio data from the `chunk` into the internal lock-free SPSC queue (`_queue`).
            -   Returns `true` on success, `false` if the internal queue is full.

    -   **Private Worker Thread Method:**
        -   `void workerLoop();`:
            -   This method runs in the `_workerThread`.
            -   Continuously:
                1.  Checks if data is available in the internal `_queue` (by comparing `_readIndex` and `_writeIndex`).
                2.  If data is available, it dequeues an `AudioChunk_POD`'s data.
                3.  Calls `_provider->pushAudioData(data_ptr, data_size, timestamp)` (timestamp might be part of `QueueItem` or passed separately if needed by the provider). The `_provider` then takes this data for AMDT-P packetization and transmission.
                4.  If the queue is empty, it sleeps for a short duration to avoid busy-waiting.

    -   **Private Members:**
        -   `FWA::Isoch::ITransmitPacketProvider* _provider = nullptr;`: Pointer to the packet provider for the active output stream.
        -   `std::thread _workerThread;`: The dedicated thread for dequeuing and pushing data to the provider.
        -   `std::atomic<bool> _running{false};`: Controls the lifecycle of the worker thread.
        -   `std::shared_ptr<spdlog::logger> _logger;`: For logging.

**Overall Role:**
The `ShmIsochBridge` is a critical component in the `FWADaemon`'s audio output path. It acts as an intermediary buffer and dispatcher:
1.  It receives audio data chunks that the `RingBufferManager` has read from the shared memory segment (data originating from the kernel driver).
2.  It temporarily stores these chunks in its own internal, lock-free SPSC queue. This helps to decouple the shared memory reading rate from the isochronous transmission rate and smooth out potential timing jitter.
3.  A dedicated worker thread in `ShmIsochBridge` continuously pulls data from this internal queue and feeds it to an `ITransmitPacketProvider`. This provider is part of the isochronous streaming stack (e.g., `IsochPacketProvider` leading to `AmdtpTransmitStreamProcessor` and `AmdtpTransmitter`) that packetizes the audio into AMDT-P format and sends it to the FireWire device.
Essentially, it bridges the shared-memory-based communication from the driver to the isochronous stream transmission logic within the daemon.
