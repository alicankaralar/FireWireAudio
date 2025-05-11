# Summary for include/FWA/DeviceInfo.hpp

This C++ header file defines the `FWA::DeviceInfo` struct (or class). This structure is a data container designed to hold a comprehensive collection of information about a specific FireWire audio device. This information is typically gathered by parsing the device's AV/C descriptors and IOKit registry properties.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The structure is defined within the `FWA` namespace.

-   **Includes:**
    -   `<string>`: For `std::string`.
    -   `<vector>`: For `std::vector`.
    -   `<cstdint>`: For `uint64_t`, `uint8_t`.
    -   `<optional>`: For `std::optional`, used for parts of the device info that might not be present (e.g., specific subunit details).
    -   `"AudioPlug.hpp"`: For the `AudioPlug` class, as device info includes lists of audio plugs.
    -   `"AudioSubunit.hpp"` (or a simplified `AudioSubunitInfo` struct): To store information specific to the Audio Subunit.
    -   `"MusicSubunit.hpp"` (or a simplified `MusicSubunitInfo` struct/`MusicSubunitCapabilities`): To store information specific to the Music (MIDI) Subunit.
    -   `<nlohmann/json.hpp>`: For JSON serialization capabilities.

-   **Struct/Class `DeviceInfo`:**
    -   **Public Member Variables (or members with public accessors):**
        -   **Basic Identification:**
            -   `guid`: `uint64_t` - The Global Unique Identifier of the device.
            -   `name`: `std::string` - The primary name of the device (often the model name).
            -   `vendorName`: `std::string` - The manufacturer's name.
            -   `modelName`: `std::string` - The specific model name.
        -   **Serial and Versioning:**
            -   `serialNumber`: `std::string` - The device's serial number.
            -   `firmwareVersion`: `std::string` - The firmware version string.
            -   `hardwareVersion`: `std::string` - The hardware version string (if available).
        -   **Plug Information:**
            -   `inputPlugs`: `std::vector<AudioPlug>` - A list of all discovered audio input plugs on the device.
            -   `outputPlugs`: `std::vector<AudioPlug>` - A list of all discovered audio output plugs on the device.
            -   (Potentially similar lists for MIDI plugs if not encapsulated within `MusicSubunitInfo`).
        -   **Subunit Specific Information:**
            -   `audioSubunitInfo`: `std::optional<AudioSubunitInfo>` (or `std::optional<FWA::AudioSubunit::ParsedInfo>`) - Contains detailed information parsed from the Audio Subunit, if one exists. This might include a list of its internal plugs or specific capabilities not covered by the top-level `AudioPlug` entries.
            -   `musicSubunitInfo`: `std::optional<MusicSubunitInfo>` (or `std::optional<FWA::MusicSubunitCapabilities>`) - Contains detailed information parsed from the Music Subunit (MIDI capabilities, MIDI plugs), if one exists.
        -   **Raw Descriptor Data (Optional Storage for Debugging/Advanced Use):**
            -   `unitInfoDescriptor`: `std::optional<std::vector<uint8_t>>` - Raw bytes of the Unit Information descriptor.
            -   `audioSubunitDescriptor`: `std::optional<std::vector<uint8_t>>` - Raw bytes of the Audio Subunit Information descriptor.
            -   `musicSubunitDescriptor`: `std::optional<std::vector<uint8_t>>` - Raw bytes of the Music Subunit Information descriptor.
            -   (Potentially other raw descriptors).

    -   **Constructor:**
        -   A default constructor is likely provided, initializing members to default states (e.g., empty strings/vectors, nullopt).

    -   **JSON Serialization (`toJson` method):**
        -   `nlohmann::json toJson() const;`
        -   This method converts the entire `DeviceInfo` structure into a `nlohmann::json` object.
        -   It iterates through all its member variables and adds corresponding key-value pairs to the JSON object.
        -   For complex members like `inputPlugs`, `outputPlugs`, `audioSubunitInfo`, and `musicSubunitInfo`, it would call their respective `toJson()` methods to get their JSON representations and include them as nested objects or arrays.
        -   Raw descriptor data, if stored and included, is typically converted to a hexadecimal string representation.

**Overall Role:**
The `DeviceInfo` structure acts as a comprehensive, aggregated snapshot of all the information the FWA library has discovered and parsed about a particular FireWire audio device.
-   It is primarily populated by the `DeviceParser` class during the device initialization phase.
-   An instance of `DeviceInfo` is typically held as a member of the `AudioDevice` class.
-   It serves as the central data store for device characteristics, making it easy for:
    -   The `AudioDevice` class to access and use this information.
    -   Higher-level application logic to query device capabilities.
    -   The information to be serialized (e.g., to JSON via `toJson()`) for display in a GUI, for logging, or for communication with other processes.
