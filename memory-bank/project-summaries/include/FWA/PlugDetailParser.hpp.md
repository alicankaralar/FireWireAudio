# Summary for include/FWA/PlugDetailParser.hpp

This C++ header file defines the `FWA::PlugDetailParser` class. This class is specialized in parsing the detailed information associated with individual audio and MIDI plugs on a FireWire device. It uses a `DescriptorReader` to access and interpret specific AV/C descriptors related to these plugs, such as stream format lists and INFO blocks for names.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<memory>`: For `std::shared_ptr`.
    -   `<expected>`: For `std::expected` error handling.
    -   `"DescriptorReader.hpp"`: For the `DescriptorReader` class, which is used to obtain parsed descriptor objects.
    -   `"AudioPlug.hpp"`: For the `AudioPlug` class, which this parser populates.
    -   `"Error.h"`: For `IOKitError`.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<cstdint>`: For `uint8_t`.
    -   (If it directly handles MIDI plugs beyond just names, it might include a `MidiPlugInfo.hpp` or similar, though often MIDI plug details are simpler and might be handled by `MusicSubunitDescriptorParser` or `DeviceParser` directly for capabilities, with this class just fetching names if applicable).

-   **Class `PlugDetailParser`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `PlugDetailParser(std::shared_ptr<DescriptorReader> reader, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the parser with a `DescriptorReader` (to get parsed descriptor objects like `AudioStreamFormat` or `AVCInfoBlock`) and a logger.
        -   **Audio Plug Detail Parsing Method:**
            -   `std::expected<void, IOKitError> parseAudioPlugDetails(AudioPlug& plugToPopulate, uint8_t subunitID) const;`
            -   **Parameters:**
                -   `plugToPopulate`: A reference to an `AudioPlug` object that will be populated with detailed information. This object would have been created during an initial plug discovery phase (e.g., by `UnitPlugDiscoverer` or `AudioSubunit::parseDescriptors`) with basic info like ID and direction.
                -   `subunitID`: The ID of the subunit to which this plug belongs.
            -   **Operation (Conceptual - details in .cpp):**
                1.  **Parse Stream Formats:** Uses the `_reader->getStreamFormats(subunitID, plugToPopulate.getID(), plugToPopulate.getDirection() == PlugDirection::Input)` to get a vector of `AudioStreamFormat` objects supported by this plug.
                2.  Adds each valid `AudioStreamFormat` to the `plugToPopulate` object using `plugToPopulate.addSupportedFormat()`.
                3.  **Parse Plug Name (INFO Block):** Uses the `_reader` (which in turn uses `DescriptorAccessor`) to attempt to read an INFO block descriptor associated with this specific plug ID on this subunit. If an `AVCInfoBlock` is successfully parsed, its text is set as the name of `plugToPopulate` using `plugToPopulate.setName()`.
                4.  (Potentially parse other audio-plug-specific descriptors like "Audio Port Descriptor" if more details are needed beyond what `AudioPlug` and `AudioStreamFormat` already cover).
                5.  Returns `std::expected<void, IOKitError>` to indicate success or failure of the detailed parsing.
        -   **MIDI Plug Detail Parsing Method (Conceptual):**
            -   `std::expected<std::string, IOKitError> parseMIDIPlugName(uint8_t subunitID, uint8_t plugID) const;` (Example: if only the name is needed for MIDI plugs from this parser).
            -   If more details for MIDI plugs were handled here, it would take a `MidiPlugInfo&` or similar to populate.

    -   **Private Members:**
        -   `std::shared_ptr<DescriptorReader> _reader;`: The descriptor reader instance.
        -   `std::shared_ptr<spdlog::logger> _logger;`: The logger instance.

**Overall Role:**
The `PlugDetailParser` is a focused utility that enriches the information about already discovered audio (and potentially MIDI) plugs. After a more general discovery phase identifies the existence and basic type of plugs, `DeviceParser` would likely use an instance of `PlugDetailParser` to:
1.  Determine the full range of audio stream formats supported by each audio plug.
2.  Retrieve human-readable names for these plugs from their associated INFO block descriptors.
This detailed information is essential for the `DeviceInfo` object to provide a complete picture of the device's I/O capabilities, allowing applications to make informed decisions about stream configuration.
