# Summary for include/FWA/AudioPlug.hpp

This C++ header file defines the `FWA::AudioPlug` class. This class is a data structure used to represent a single audio plug (either an input or an output) on a FireWire audio device. It encapsulates information about the plug's identity, direction, channel count, name, and the audio formats it supports.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<string>`: For `std::string`.
    -   `<vector>`: For `std::vector`.
    -   `<cstdint>`: For `uint8_t`.
    -   `"Enums.hpp"`: For the `PlugDirection` enum (which would define `Input` and `Output`).
    -   `"AudioStreamFormat.hpp"`: For the `AudioStreamFormat` struct/class, which describes a specific audio format (sample rate, bit depth, etc.).
    -   `<nlohmann/json.hpp>`: For JSON serialization capabilities.

-   **Class `AudioPlug`:**
    -   **Public Member Variables (or members with public accessors/mutators):**
        -   `_id`: `uint8_t` - The identifier for this plug, as defined by the device's descriptors.
        -   `_direction`: `PlugDirection` - Indicates whether the plug is an input or an output.
        -   `_numChannels`: `uint8_t` - The number of audio channels this plug handles (e.g., 1 for mono, 2 for stereo).
        -   `_name`: `std::string` - A human-readable name for the plug, often obtained from an AV/C INFO block descriptor.
        -   `_supportedFormats`: `std::vector<AudioStreamFormat>` - A list of all audio stream formats that this plug supports.

    -   **Constructor:**
        -   `AudioPlug(uint8_t id, PlugDirection direction, uint8_t numChannels, std::string name = "");`
        -   Initializes an `AudioPlug` object with its ID, direction, number of channels, and an optional name. The `_supportedFormats` vector would be initially empty.

    -   **Public Methods:**
        -   **Accessors (Getters):**
            -   `uint8_t getID() const;`
            -   `PlugDirection getDirection() const;`
            -   `uint8_t getNumChannels() const;`
            -   `const std::string& getName() const;`
            -   `const std::vector<AudioStreamFormat>& getSupportedFormats() const;`
        -   **Mutators (Setters):**
            -   `void setName(const std::string& name);`
            -   `void addSupportedFormat(const AudioStreamFormat& format);` (Used by the parser to populate the list of supported formats).
        -   **Format Checking:**
            -   `bool isFormatSupported(const AudioStreamFormat& format) const;`: Checks if a given `AudioStreamFormat` is present in the `_supportedFormats` list.
        -   **JSON Serialization:**
            -   `nlohmann::json toJson() const;`: Converts the `AudioPlug` object's state (ID, direction, name, number of channels, and the list of supported formats) into a `nlohmann::json` object. Each `AudioStreamFormat` in `_supportedFormats` would also be serialized to its JSON representation.

**Overall Role:**
The `AudioPlug` class provides a structured way to store and access information about an individual audio input or output point on a FireWire device.
-   It is populated by parser classes (like `DeviceParser` and `PlugDetailParser`) during the device discovery and descriptor parsing phase.
-   Instances of `AudioPlug` are typically stored within the `DeviceInfo` object (e.g., in `inputPlugs` and `outputPlugs` vectors).
-   This information is crucial for:
    -   The FWA library to understand the device's I/O capabilities.
    -   Applications using the library to select appropriate plugs and formats for audio streaming.
    -   Serializing device information (e.g., to JSON for display in a GUI or for debugging).
