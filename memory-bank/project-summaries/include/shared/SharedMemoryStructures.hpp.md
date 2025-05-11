# Summary for include/shared/SharedMemoryStructures.hpp

This C++ header file defines the data structures and utility functions for a lock-free SPSC (Single-Producer, Single-Consumer) ring buffer. This ring buffer is designed to be placed in a POSIX shared memory segment to facilitate efficient, low-latency transfer of audio data chunks from the kernel-level audio driver (producer) to the user-space `FWADaemon` (consumer).

**Key Declarations and Components:**

-   **Namespace `RTShmRing` (Real-Time Shared Memory Ring):**
    -   All definitions are encapsulated within this namespace.

-   **Includes:**
    -   `<cstdint>`: For fixed-width integer types (`uint8_t`, `uint32_t`, `uint64_t`).
    -   `<atomic>`: For `std::atomic`, used for the lock-free read and write counters.
    -   `<cstring>`: For `memcpy`.
    -   (No external library dependencies beyond the C++ standard library).

-   **Constants:**
    -   `static constexpr uint32_t kRingCapacityPow2 = 256;`: Defines the capacity of the ring buffer. It must be a power of 2 for the modulo arithmetic (`% kRingCapacityPow2`) to be efficiently implemented as a bitwise AND (`& (kRingCapacityPow2 - 1)`).
    -   `static constexpr uint32_t kMaxAudioDataBytes = 4096;`: Defines the maximum size of the audio data payload within a single `AudioChunk_POD`. This determines the size of the `audio` array in the chunk.
    -   `static constexpr uint32_t kShmVersion = 0x00010001;`: An ABI (Application Binary Interface) version number for the shared memory structures. This allows the producer and consumer to verify they are using compatible layouts.

-   **`AudioChunk_POD` Struct:**
    -   A Plain Old Data (POD) struct representing a single chunk of audio data transferred through the ring buffer.
    -   `timestamp`: `uint64_t` - Typically a host time (e.g., `mach_absolute_time()`) indicating when the audio data was captured or is scheduled for playback.
    -   `dataBytes`: `uint32_t` - The actual number of valid audio bytes in the `audio` buffer for this chunk.
    -   `audio`: `uint8_t[kMaxAudioDataBytes]` - A fixed-size character array to hold the raw audio sample data.

-   **`ControlBlock_POD` Struct:**
    -   A POD struct containing the control variables for the SPSC ring buffer. These are critical for the lock-free algorithm.
    -   `abiVersion`: `uint32_t` - Stores the ABI version of the shared memory layout.
    -   `capacity`: `uint32_t` - Stores the capacity of the ring buffer (should match `kRingCapacityPow2`).
    -   `readCount`: `std::atomic<uint64_t>` - The total number of chunks successfully read (popped) by the consumer. `std::memory_order_acquire` and `std::memory_order_relaxed` are used for memory ordering.
    -   `writeCount`: `std::atomic<uint64_t>` - The total number of chunks successfully written (pushed) by the producer. `std::memory_order_release` and `std::memory_order_relaxed` are used.

-   **`SharedRingBuffer_POD` Struct:**
    -   The top-level POD struct that defines the entire layout of the shared memory segment.
    -   `control`: `ControlBlock_POD` - An instance of the control block.
    -   `ring`: `AudioChunk_POD[kRingCapacityPow2]` - The actual array of `AudioChunk_POD` objects that forms the ring buffer.

-   **Inline Lock-Free SPSC Queue Functions:**
    -   **`push(ControlBlock_POD& cb, AudioChunk_POD ring[], const AudioChunk_POD& chunk)`:**
        -   The producer-side function to add a chunk to the ring buffer.
        -   It checks if the buffer is full (`cb.writeCount - cb.readCount.load(std::memory_order_acquire) >= cb.capacity`).
        -   If not full, it copies the `chunk` data into the `ring` at the current write position (`cb.writeCount % cb.capacity`).
        -   Atomically increments `cb.writeCount` using `std::memory_order_release` to make the write visible to the consumer.
        -   Returns `true` on success, `false` if the buffer was full.
    -   **`pop(ControlBlock_POD& cb, AudioChunk_POD ring[], AudioChunk_POD& outChunk)`:**
        -   The consumer-side function to remove a chunk from the ring buffer.
        -   It checks if the buffer is empty (`cb.readCount == cb.writeCount.load(std::memory_order_acquire)`).
        -   If not empty, it copies the data from the `ring` at the current read position (`cb.readCount % cb.capacity`) into `outChunk`.
        -   Atomically increments `cb.readCount` using `std::memory_order_release` (though `relaxed` might be sufficient here if only one consumer).
        -   Returns `true` on success, `false` if the buffer was empty.

**Overall Role:**
This header file is absolutely critical for the low-latency, real-time audio data transfer between the kernel-mode audio driver (producer) and the user-space `FWADaemon` (consumer).
-   It defines a shared memory layout that both processes can agree upon.
-   It provides a lock-free SPSC queue implementation, which is highly efficient for single-producer, single-consumer scenarios, avoiding the overhead and priority inversion issues associated with traditional locks (mutexes, semaphores) in a real-time audio context.
The driver writes `AudioChunk_POD`s into this shared memory ring buffer, and the `RingBufferManager` within the `FWADaemon` reads from it using these `push` and `pop` semantics.
