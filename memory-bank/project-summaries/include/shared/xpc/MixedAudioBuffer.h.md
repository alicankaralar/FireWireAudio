# Summary for include/shared/xpc/MixedAudioBuffer.h

This Objective-C header file declares the interface for the `MixedAudioBuffer` class. This class is designed as a container to hold a chunk of audio data along with its associated metadata (format, timestamp, frame count), specifically for the purpose of sending it over XPC (Cross-Process Communication). This file is identical in content and purpose to `include/FWA/XPC/MixedAudioBuffer.h`.

**Key Declarations and Components:**

-   **`@import Foundation;`**: Imports the Foundation framework, providing base Objective-C classes like `NSObject` and `NSData`, and the `NSSecureCoding` protocol.
-   **`@import CoreAudio;`**: Imports the CoreAudio framework, specifically for `CoreAudioTypes.h` which defines `AudioStreamBasicDescription` (ASBD) and `AudioTimeStamp`.

-   **Class `MixedAudioBuffer`:**
    -   **Inheritance:** `NSObject`
    -   **Protocol Conformance:** `NSSecureCoding`
        -   This is crucial for XPC. It means instances of `MixedAudioBuffer` can be securely encoded and decoded when sent across process boundaries. The actual encoding/decoding logic is implemented in the corresponding `.m` file (likely `src/FWA/XPC/MixedAudioBuffer.m`).

    -   **Properties (Declared `nonatomic`, `readonly`):**
        -   `@property (nonatomic, strong, readonly) NSData *audioData;`
            -   Stores the raw audio sample data.
        -   `@property (nonatomic, assign, readonly) AudioStreamBasicDescription format;`
            -   An `AudioStreamBasicDescription` (ASBD) struct from Core Audio, describing the audio format (sample rate, channels, bit depth, format flags, etc.).
        -   `@property (nonatomic, assign, readonly) uint64_t timestamp;`
            -   A timestamp for the audio data, likely an `AudioTimeStamp.mHostTime`.
        -   `@property (nonatomic, assign, readonly) uint32_t frameCount;`
            -   The number of audio frames in `audioData`.

    -   **Initializers:**
        -   `- (instancetype)initWithData:(NSData *)data format:(AudioStreamBasicDescription)format timestamp:(uint64_t)timestamp frameCount:(uint32_t)frameCount NS_DESIGNATED_INITIALIZER;`
            -   The designated initializer.
        -   `- (instancetype)init NS_UNAVAILABLE;`
            -   Disables the default `init` method.
        -   `+ (instancetype)new NS_UNAVAILABLE;`
            -   Disables the default `new` method.

**Overall Role:**
The `MixedAudioBuffer` class provides a standardized, XPC-safe way to encapsulate and transmit blocks of audio data and its critical descriptive metadata.
-   It ensures that when audio data is passed between processes (e.g., from the `FWADaemon` to a client GUI for metering, or potentially from a client to the daemon), the receiving process has all the necessary information (format, timestamp, frame count) to correctly interpret and use the audio samples.
-   Its conformance to `NSSecureCoding` is vital for secure and reliable serialization/deserialization when used as an argument or return type in XPC messages.
This class is likely used in the XPC protocols (e.g., `FWAClientNotificationProtocol`) where audio data needs to be exchanged between the `FWADaemon` and its clients.
