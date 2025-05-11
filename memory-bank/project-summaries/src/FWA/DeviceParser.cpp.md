# Summary for src/FWA/DeviceParser.cpp

This C++ file implements the `FWA::DeviceParser` class. The main purpose of this class is to comprehensively parse the AV/C (Audio Video Control) descriptors of a FireWire audio device to build a structured representation of its capabilities, primarily populating a `DeviceInfo` object.

**Key Functionalities:**

-   **Constructor (`DeviceParser::DeviceParser`):**
    -   Takes a `std::shared_ptr<DescriptorAccessor>` (used to read raw descriptor data), a `std::shared_ptr<CommandInterface>` (though descriptor reading is mostly delegated to the accessor, this might be kept for other direct commands if needed or for the accessor's own initialization), and an `std::shared_ptr<spdlog::logger>`.
    -   Stores these dependencies.

-   **Main Parsing Method (`parse(DeviceInfo& deviceInfo, const Subunit& audioSubunit, const Subunit& musicSubunit)`):**
    -   This is the orchestrating method that drives the parsing process.
    -   It takes a `DeviceInfo` object by reference, which it will populate with the parsed data.
    -   It also takes references to `AudioSubunit` and `MusicSubunit` objects (which would have been discovered by `SubunitDiscoverer` and passed to `AudioDevice`).
    -   **Steps:**
        1.  **Parse Unit Information:** Calls a private helper method like `parseUnitInformation(deviceInfo)` which uses the `_descriptorAccessor` to read the main "Unit Information" descriptor. This typically contains:
            -   Vendor ID, Model ID.
            -   Unit capabilities.
            -   Pointers/references to INFO blocks for model name, vendor name, etc. These INFO blocks are then read and parsed (using `AVCInfoBlock`) to populate `deviceInfo.modelName`, `deviceInfo.vendorName`.
        2.  **Parse Audio Subunit Information (if audio subunit exists):**
            -   Calls a helper like `parseAudioSubunitDetails(deviceInfo, audioSubunit)`.
            -   Uses `_descriptorAccessor` to read descriptors specific to the audio subunit (e.g., `readAudioPlugInfoList`, `readStreamFormatInfoList`).
            -   For each audio plug descriptor, it creates an `AudioPlug` object, populating its ID, direction, channel count.
            -   It then reads the stream format list for that plug, creating `AudioStreamFormat` objects for each supported format and associating them with the `AudioPlug`.
            -   It may also read INFO blocks associated with audio plugs to get their names.
            -   Populates `deviceInfo.inputPlugs` and `deviceInfo.outputPlugs`.
        3.  **Parse Music Subunit Information (if music subunit exists):**
            -   Calls a helper like `parseMusicSubunitDetails(deviceInfo, musicSubunit)`.
            -   Uses `_descriptorAccessor` to read descriptors for the MIDI (Music) subunit, such as `readMusicPlugInfoList`.
            -   Parses MIDI plug information (e.g., number of MIDI IN/OUT jacks).
            -   May read `MusicSubunitCapabilitiesDescriptor` to understand features like SysEx support, hardware MIDI thru, etc.
            -   Populates relevant fields in `deviceInfo` or a nested `MusicSubunitInfo` struct within `deviceInfo`.
        4.  **(Potentially) Parse other common or vendor-specific descriptors.**

-   **Private Helper Parsing Methods:**
    -   The class contains various private methods, each dedicated to parsing a specific type or category of descriptor (e.g., `parseUnitInfoDescriptor`, `parseAudioPlugDescriptorList`, `parseStreamFormatDescriptorList`, `parseInfoBlock`).
    -   These methods make extensive use of:
        -   `_descriptorAccessor` to fetch the raw descriptor byte vectors.
        -   `DescriptorUtils` to parse common fields (like subunit identifiers, plug IDs, SFCs) from the raw byte vectors.
        -   `AVCInfoBlock` to parse textual INFO blocks.
    -   They handle the logic of iterating through lists of descriptors and populating the corresponding fields in the `DeviceInfo` object or its constituent parts (like `AudioPlug` and `AudioStreamFormat` objects).

-   **Error Handling:**
    -   The parsing methods typically return `std::expected` or use internal checks to handle cases where descriptors are missing, malformed, or AV/C commands fail. Errors are logged using the provided logger.

**Overall Role:**
The `DeviceParser` is a crucial component for understanding a FireWire audio device. It acts as a high-level coordinator that uses the `DescriptorAccessor` to retrieve raw descriptor data and then applies parsing logic (often aided by `DescriptorUtils` and `AVCInfoBlock`) to transform this raw data into a structured `DeviceInfo` object. This `DeviceInfo` object then serves as the FWA library's internal model of the device's capabilities, which is used by `AudioDevice` and can be exposed to applications (e.g., as JSON).
