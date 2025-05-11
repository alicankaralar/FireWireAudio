# Summary for include/shared/xpc/RingBufferManager.hpp

This C++ header file defines the `RingBufferManager` class. This class is designed as a singleton to manage the mapping and access to the POSIX shared memory segment that contains the `RTShmRing::SharedRingBuffer_POD`. It's intended to be used by the `FWADaemon` to interact with the shared memory region established for communication with the kernel driver.

**Key Declarations and Components:**

-   **Includes:**
    -   `"../SharedMemoryStructures.hpp"`: For the definitions of `RTShmRing::SharedRingBuffer_POD`, `RTShmRing::ControlBlock_POD`, `RTShmRing::AudioChunk_POD`, and the `push`/`pop` utility functions.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<atomic>`: For `std::atomic`.
    -   `<thread>`: For `std::thread` (if the manager itself runs a processing thread, as seen in `FWADaemon/RingBufferManager.mm`).
    -   `<mutex>`: For synchronization if needed, though the primary ring buffer is lock-free.
    -   `<condition_variable>`: Potentially for signaling between threads.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<string>`: For `std::string`.

-   **Class `RingBufferManager`:**
    -   **Singleton Access:**
        -   `static RingBufferManager& instance();`: Provides global access to the single instance of `RingBufferManager`.
    -   **Constructor and Destructor (Private/Deleted):**
        -   `RingBufferManager();`
        -   `~RingBufferManager();`
        -   Copy and move constructors/assignment operators are deleted to enforce the singleton pattern.
    -   **Public Interface:**
        -   **Mapping and Unmapping Shared Memory:**
            -   `bool map(int shmFd, bool isCreator);`:
                -   **Parameters:**
                    -   `shmFd`: The file descriptor of the opened POSIX shared memory segment.
                    -   `isCreator`: A boolean flag indicating whether the calling process (i.e., the `FWADaemon`) is responsible for creating and initializing the shared memory segment.
                -   **Functionality (Conceptual - implementation in .mm):**
                    -   Maps the shared memory segment into the process's address space using `mmap()`.
                    -   Stores the pointer to the mapped `RTShmRing::SharedRingBuffer_POD`.
                    -   If `isCreator`, it initializes the `ControlBlock_POD` (e.g., sets ABI version, capacity, zeroes read/write counts).
                    -   If not `isCreator`, it verifies the ABI version and capacity of an existing segment.
                    -   May attempt to `mlock()` the memory.
                    -   May start an internal reader thread if this manager is responsible for consuming data from the ring buffer (as is the case in `FWADaemon/RingBufferManager.mm`).
            -   `void unmap();`:
                -   Stops any internal threads.
                -   Unlocks (`munlock()`) and unmaps (`munmap()`) the shared memory segment.
                -   Resets internal state.
        -   **Access to Shared Structures (Conceptual - might be private if only used internally):**
            -   `RTShmRing::SharedRingBuffer_POD* getSharedRingBuffer() const;`: Returns a pointer to the mapped shared ring buffer structure.
            -   `RTShmRing::ControlBlock_POD* getControlBlock() const;`: Returns a pointer to the control block within the shared memory.
        -   **Reader Thread Control (If applicable, as in the daemon's version):**
            -   `void startReaderThread();` (Potentially called by `map`)
            -   `void stopReaderThread();` (Potentially called by `unmap`)

    -   **Private Members (Conceptual - implementation in .mm):**
        -   `_shmFd`: `int` - The file descriptor for the shared memory.
        -   `_shmPtr`: `RTShmRing::SharedRingBuffer_POD*` - Pointer to the mapped shared memory.
        -   `_isCreator`: `bool`.
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.
        -   `_isRunning`: `std::atomic<bool>` (if it runs an internal thread).
        -   `_readerThread`: `std::thread` (if it runs an internal thread).
        -   (Other state variables as needed for managing the shared memory lifecycle).

**Overall Role:**
The `RingBufferManager` class (as declared in this shared header) provides the essential interface for a process to attach to and manage its view of the shared memory ring buffer defined by `SharedMemoryStructures.hpp`.
-   In the context of the `FWADaemon`, its implementation (`src/xpc/FWADaemon/RingBufferManager.mm`) uses this to map the shared memory created by the daemon and then runs a reader thread to continuously `pop` audio data (produced by the kernel driver) and forward it to the `ShmIsochBridge` for isochronous transmission.
-   The kernel driver's user-space component (`DriverXPCManager`) would also conceptually use these structures (though it might have its own mapping logic) to act as the producer, `push`ing audio data into the shared memory.
This class, along with `SharedMemoryStructures.hpp`, forms the core of the inter-process communication mechanism for audio data between the driver and the daemon.
