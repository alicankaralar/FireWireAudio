# Summary for include/FWA/AudioStreamFormat.hpp

This C++ header file defines the `FWA::AudioStreamFormat` struct. This structure is a plain data object used throughout the FWA library to describe the specific format of an audio stream, including its sample rate, bit depth, number of channels, and sample type.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The struct is defined within the `FWA` namespace.

-   **Includes:**
    -   `<cstdint>`: For fixed-width integer types like `uint8_t`.
    -   `<string>`: For `std::string` (used in `toString()`).
    -   `<nlohmann/json.hpp>`: For JSON serialization capabilities.
    -   `<sstream>`: For `std::ostringstream` (used in `toString()`).

-   **Struct `AudioStreamFormat`:**
    -   **Public Member Variables:**
        -   `sampleRate`: `double` - The sample rate in Hertz (e.g., 44100.0, 48000.0, 96000.0).
        -   `bitDepth`: `uint8_t` - The number of bits per audio sample (e.g., 16, 24, 32).
        -   `numChannels`: `uint8_t` - The number of audio channels (e.g., 1 for mono, 2 for stereo).
        -   `isFloat`: `bool` - Indicates if the audio samples are floating-point (`true`) or integer (`false`).
        -   `isInterleaved`: `bool` - Indicates if the audio data for multiple channels is interleaved (`true`) or non-interleaved (planar, `false`). Defaults to `true`.

    -   **Constructor:**
        -   `AudioStreamFormat(double sr = 0.0, uint8_t bd = 0, uint8_t chans = 0, bool flt = false, bool interleaved = true);`
        -   Provides a constructor with default values for all members, allowing for easy initialization.

    -   **Comparison Operators:**
        -   `bool operator==(const AudioStreamFormat& other) const;`: Defines equality comparison based on all member variables.
        -   `bool operator!=(const AudioStreamFormat& other) const;`: Defines inequality comparison.

    -   **JSON Serialization (`toJson`):**
        -   `nlohmann::json toJson() const;`:
            -   Converts the `AudioStreamFormat` object's state (all its member variables) into a `nlohmann::json` object.
            -   Each member variable becomes a key-value pair in the resulting JSON.

    -   **String Representation (`toString`):**
        -   `std::string toString() const;`:
            -   Creates and returns a human-readable string representation of the audio format (e.g., "48000 Hz, 24-bit, 2 ch, Int, Interleaved").

**Overall Role:**
The `AudioStreamFormat` struct is a fundamental data structure used extensively within the FWA library to:
-   Describe the capabilities of audio plugs on a device (i.e., the list of formats an `AudioPlug` supports).
-   Specify the desired format when creating an `AudioDeviceStream`.
-   Represent the current operating format of an active `AudioDeviceStream`.
-   Communicate format information between different components of the library (e.g., from `DeviceParser` to `DeviceInfo`, and then to `AudioDevice`).
-   Potentially exchange format information with the Core Audio driver via the `FWADaemon` and shared memory structures.
Its clear and concise definition, along with comparison and serialization methods, makes it easy to work with audio format descriptions.
