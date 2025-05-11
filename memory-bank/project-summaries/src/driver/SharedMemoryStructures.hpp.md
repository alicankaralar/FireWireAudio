# Summary for src/driver/SharedMemoryStructures.hpp

This C++ header file defines the data structures and core logic for a lock-free SPSC (Single-Producer, Single-Consumer) ring buffer. This ring buffer is designed to be placed in a shared memory segment to facilitate efficient, low-latency transfer of audio data between the kernel-space driver (producer) and a user-space daemon (consumer).

**Key Components and Functionalities:**

-   **Namespace `RTShmRing`:** Encapsulates all related definitions.

-   **Constants:**
    -   `kShmVersion`: An integer defining the ABI (Application Binary Interface) version of the shared memory structures. This helps in ensuring compatibility between the driver and daemon if the structures evolve.
    -   `kRingCapacityPow2`: Defines the capacity of the ring buffer data area in bytes. It **must** be a power of two for the bitwise AND trick (`& (capacity - 1)`) used in the SPSC algorithm for efficient index wrapping.

-   **`AudioSample` Typedef:**
    -   `using AudioSample = float;`: Defines the type of individual audio samples as `float`.

-   **`AudioChunk_POD` Struct:**
    -   A Plain Old Data (POD) structure designed to hold a chunk of audio data.
    -   `frameCount`: `uint32_t` indicating the number of audio frames in this chunk.
    -   `timeStamp`: `AbsoluteTime` (a macOS kernel time type) indicating the presentation timestamp for this chunk.
    -   `audio[]`: A flexible array member (C99 feature, also works in C++) intended to hold interleaved stereo audio samples (`AudioSample L, AudioSample R, ...`). The actual size of this array depends on `frameCount`.
    -   `static size_t calculateSize(uint32_t frameCount)`: A static method to calculate the total byte size of an `AudioChunk_POD` instance given a specific `frameCount`.

-   **`ControlBlock_POD` Struct:**
    -   A POD structure for the control block of the shared ring buffer. This part contains metadata for managing the buffer.
    -   `abiVersion`: `uint32_t` storing the version of the shared memory layout.
    -   `capacity`: `uint32_t` storing the total capacity of the `ring[]` data buffer in bytes (must be `kRingCapacityPow2`).
    -   `writeCount`: `std::atomic<uint64_t>` representing the total number of bytes ever written (or intended to be written). Used by the producer.
    -   `readCount`: `std::atomic<uint64_t>` representing the total number of bytes ever read. Used by the consumer.
    -   The use of `std::atomic` is crucial for the lock-free nature of the SPSC queue.

-   **`SharedRingBuffer_POD` Struct:**
    -   The main POD structure that defines the entire layout of the shared memory segment.
    -   `control`: An instance of `ControlBlock_POD`.
    -   `ring[]`: A flexible array member (`uint8_t`) representing the actual circular data buffer. Its size is effectively `kRingCapacityPow2`.

-   **`push(ControlBlock_POD& ctrl, uint8_t ring[], const AudioChunk_POD& chunk)` Inline Function:**
    -   The **producer-side** function to write an `AudioChunk_POD` into the ring buffer.
    -   Implements a lock-free SPSC algorithm:
        -   Calculates current write and read positions using `writeCount` and `readCount`.
        -   Checks if there's enough space in the buffer for the chunk (including its size prefix).
        -   If space is available:
            -   Writes the size of the chunk (4 bytes) to the ring buffer.
            -   Writes the actual `AudioChunk_POD` data (header + audio samples) to the ring buffer.
            -   Handles buffer wrapping using bitwise AND with `(ctrl.capacity - 1)`.
            -   Atomically updates `ctrl.writeCount` using `std::memory_order_release` to ensure memory operations are visible to the consumer.
        -   Returns `true` on success, `false` if the buffer is full.

-   **`pop(ControlBlock_POD& ctrl, const uint8_t ring[], AudioChunk_POD& chunk)` Inline Function:**
    -   The **consumer-side** function to read an `AudioChunk_POD` from the ring buffer.
    -   Implements the corresponding lock-free SPSC algorithm:
        -   Calculates current write and read positions.
        -   Checks if there's data available to read.
        -   If data is available:
            -   Reads the size of the next chunk (4 bytes) from the ring buffer.
            -   Reads the actual `AudioChunk_POD` data into the provided `chunk` reference.
            -   Handles buffer wrapping.
            -   Atomically updates `ctrl.readCount` using `std::memory_order_release`.
        -   Returns `true` on success, `false` if the buffer is empty.

**Overall Role:**
This header file is fundamental for inter-process communication (IPC) between the kernel driver and the user-space daemon. It defines a highly efficient, lock-free, single-producer, single-consumer queue tailored for streaming audio data with minimal latency. The driver acts as the producer, pushing `AudioChunk_POD`s, and the daemon acts as the consumer, popping them. The use of POD structures ensures a well-defined memory layout for shared memory.
