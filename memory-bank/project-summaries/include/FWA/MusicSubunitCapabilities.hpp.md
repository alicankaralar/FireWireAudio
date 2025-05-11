# Summary for include/FWA/MusicSubunitCapabilities.hpp

This C++ header file defines the `FWA::MusicSubunitCapabilities` class. This class serves as a data structure to store and provide access to the various MIDI-related capabilities of an AV/C Music Subunit. This information is typically obtained by parsing the "Music Subunit Capabilities Descriptor" read from the device.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<vector>`: For `std::vector<uint8_t>` (used in `setCapabilitiesFromData`).
    -   `<cstdint>`: For `uint8_t`.
    -   `<nlohmann/json.hpp>`: For JSON serialization.

-   **Class `MusicSubunitCapabilities`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `MusicSubunitCapabilities();`
            -   Initializes all boolean capability flags to `false` and MIDI plug counts to `0`.
        -   **Parsing Method:**
            -   `void setCapabilitiesFromData(const std::vector<uint8_t>& data);`
            -   This method is crucial. It takes a `std::vector<uint8_t>` containing the raw byte data of a Music Subunit Capabilities Descriptor (typically 8 bytes long).
            -   It then parses the individual bits and bytes from this `data` vector according to the AV/C specification for this descriptor to set the corresponding boolean flags and plug count members of the `MusicSubunitCapabilities` object.
        -   **Accessor Methods (Getters):**
            -   A comprehensive set of `const` getter methods are provided to query each capability:
                -   `bool supportsMidiIn() const;`
                -   `bool supportsMidiOut() const;`
                -   `uint8_t getNumberOfMidiInPlugs() const;`
                -   `uint8_t getNumberOfMidiOutPlugs() const;`
                -   `bool supportsProgrammableSystemExclusive() const;`
                -   `bool supportsBroadcastSystemExclusive() const;`
                -   `bool supportsIdentifiedSystemExclusive() const;`
                -   `bool supportsMidiTimeCodeIn() const;`
                -   `bool supportsMidiTimeCodeOut() const;`
                -   `bool supportsMidiMachineControlIn() const;`
                -   `bool supportsMidiMachineControlOut() const;`
                -   `bool supportsHardwareMidiThru() const;`
                -   `bool supportsGeneralMidiSystem1() const;`
                -   `bool supportsGeneralMidiSystem2() const;`
                -   `bool supportsGsExtensions() const;`
                -   `bool supportsXgExtensions() const;`
                -   (Potentially other specific GM extensions or similar flags).
        -   **JSON Serialization:**
            -   `nlohmann::json toJson() const;`: Converts all the stored capability flags and plug counts into a `nlohmann::json` object. Each capability becomes a key-value pair in the JSON.

    -   **Private Members:**
        -   A series of boolean member variables (e.g., `_supportsMidiIn`, `_supportsProgrammableSystemExclusive`, etc.) to store the state of each capability.
        -   `_numberOfMidiInPlugs`: `uint8_t`.
        -   `_numberOfMidiOutPlugs`: `uint8_t`.

**Overall Role:**
The `MusicSubunitCapabilities` class is a specialized data container that provides a structured and easily queryable representation of a Music Subunit's MIDI features.
-   It is populated by the `MusicSubunitDescriptorParser` (or directly by `MusicSubunit`'s `parseDescriptors` method) after reading and processing the raw "Music Subunit Capabilities Descriptor" from the device.
-   Once populated, it allows other parts of the FWA library (like the `MusicSubunit` class itself, or higher-level application logic via `DeviceInfo`) to determine what MIDI functionalities the device supports (e.g., how many MIDI IN/OUT ports it has, whether it supports SysEx, MTC, MMC, etc.).
-   This information is essential for correctly configuring MIDI I/O, interacting with the device's MIDI features, and presenting these capabilities to the user.
