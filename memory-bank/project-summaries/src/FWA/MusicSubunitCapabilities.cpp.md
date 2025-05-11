# Summary for src/FWA/MusicSubunitCapabilities.cpp

This C++ file implements the `FWA::MusicSubunitCapabilities` class. This class is a data structure designed to store and provide access to the capabilities of an AV/C Music Subunit, specifically those related to MIDI functionality. The information is typically parsed from the "Music Subunit Capabilities Descriptor" read from the device.

**Key Functionalities and Data Members:**

-   **Data Storage (Member Variables):**
    -   The class primarily consists of boolean flags representing various MIDI capabilities and integer counts for MIDI plugs. These members are typically named to reflect the fields in the AV/C Music Subunit Capabilities Descriptor. Examples include:
        -   `_supportsMidiIn`: bool
        -   `_supportsMidiOut`: bool
        -   `_numberOfMidiInPlugs`: uint8_t
        -   `_numberOfMidiOutPlugs`: uint8_t
        -   `_supportsProgrammableSystemExclusive`: bool
        -   `_supportsBroadcastSystemExclusive`: bool
        -   `_supportsIdentifiedSystemExclusive`: bool
        -   `_supportsMidiTimeCodeIn`: bool
        -   `_supportsMidiTimeCodeOut`: bool
        -   `_supportsMidiMachineControlIn`: bool
        -   `_supportsMidiMachineControlOut`: bool
        -   `_supportsHardwareMidiThru`: bool
        -   `_supportsGeneralMidiSystem1`: bool
        -   `_supportsGeneralMidiSystem2`: bool
        -   `_supportsGsExtensions`: bool
        -   `_supportsXgExtensions`: bool
        -   `_supportsGmExtensions`: bool (likely a typo for a more specific GM extension or a general flag)

-   **Constructor (`MusicSubunitCapabilities::MusicSubunitCapabilities`):**
    -   Initializes all boolean capability flags to `false` and plug counts to `0` by default.

-   **Parsing Logic (`setCapabilitiesFromData`):**
    -   `void setCapabilitiesFromData(const std::vector<uint8_t>& data)`:
        -   This is a crucial method that takes the raw byte data of a Music Subunit Capabilities Descriptor (typically 8 bytes long as per the AV/C specification).
        -   It parses the individual bits and bytes from this `data` vector to set the corresponding boolean flags and plug count members of the `MusicSubunitCapabilities` object.
        -   For example:
            -   Byte 0: `number_of_midi_in_plugs` and `number_of_midi_out_plugs`.
            -   Byte 1 (`midi_protocol_capabilities_1`): Bits in this byte indicate support for programmable SysEx, broadcast SysEx, identified SysEx, MIDI Time Code In/Out, MIDI Machine Control In/Out.
            -   Byte 2 (`midi_protocol_capabilities_2`): Bits indicate support for hardware MIDI Thru, General MIDI System 1/2, GS extensions, XG extensions, etc.
        -   The method carefully extracts these bits and updates the member variables.

-   **Accessor Methods (Getters):**
    -   Provides a comprehensive set of getter methods for each stored capability flag and plug count (e.g., `supportsMidiIn()`, `getNumberOfMidiOutPlugs()`, `supportsProgrammableSystemExclusive()`).

-   **Mutator Methods (Setters - Potentially Private or for Internal Use):**
    -   While getters are public, setters for individual capabilities might be private or protected if the intention is for the capabilities to be solely populated by the `setCapabilitiesFromData` method.

-   **JSON Serialization (`toJson() const`):**
    -   Converts all the stored capability flags and plug counts into a `nlohmann::json` object.
    -   Each capability becomes a key-value pair in the JSON object (e.g., `"supportsMidiIn": true`, `"numberOfMidiOutPlugs": 2`).
    -   This is used by `MusicSubunit::toJson()` and ultimately by `AudioDevice::getDeviceInfoAsJson()`.

**Overall Role:**
The `MusicSubunitCapabilities` class serves as a structured container for the MIDI-related features of a Music Subunit. It is populated by parsing the device's Music Subunit Capabilities Descriptor. Once populated, it allows other parts of the FWA library (like `MusicSubunit` or higher-level application logic) to easily query what MIDI functionalities the device supports, which is essential for configuring MIDI I/O and interacting with the device correctly.
