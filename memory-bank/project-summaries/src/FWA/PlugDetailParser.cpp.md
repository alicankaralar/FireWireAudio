# Summary for src/FWA/PlugDetailParser.cpp

This C++ file implements the `FWA::PlugDetailParser` class. This class is dedicated to parsing detailed information about individual audio and MIDI plugs on a FireWire device. It achieves this by reading and interpreting various AV/C descriptors specifically associated with plugs, using a `DescriptorAccessor`.

**Key Functionalities:**

-   **Constructor (`PlugDetailParser::PlugDetailParser`):**
    -   Takes a `std::shared_ptr<DescriptorAccessor>` (for reading raw descriptor data) and an `std::shared_ptr<spdlog::logger>`.
    -   Stores these dependencies.

-   **Audio Plug Detail Parsing (`parseAudioPlugDetails`):**
    -   `std::expected<void, IOKitError> parseAudioPlugDetails(AudioPlug& plug, uint8_t subunitID)`:
        -   Takes an `AudioPlug` object by reference (which it will populate) and the `subunitID` to which the plug belongs.
        -   **Read Audio Port Descriptor:** Uses `_descriptorAccessor->readAudioPortDescriptor(subunitID, plug.getID())` (assuming such a method exists in `DescriptorAccessor` or is built from a generic `readDescriptor` call with the correct `DescriptorSpecifier`). This descriptor provides low-level details about the audio port.
        -   **Read Stream Format Descriptor List:**
            -   Calls `_descriptorAccessor->readStreamFormatInfoList(subunitID, plug.getID(), plug.getDirection() == PlugDirection::Input)`.
            -   Iterates through the returned list of raw stream format descriptors.
            -   For each raw descriptor, it parses it to create an `AudioStreamFormat` object (likely using `DescriptorUtils::parseAudioSignalFormat` or similar helpers to interpret SFC, bit depth, etc.).
            -   Adds each valid `AudioStreamFormat` to the `plug` object using `plug.addSupportedFormat()`.
        -   **Read INFO Block for Plug Name:**
            -   Attempts to read an INFO block descriptor associated with this plug to get its textual name. This involves constructing a `DescriptorSpecifier` for an INFO block, possibly using the plug ID or a specific constant field value that denotes plug names.
            -   If an INFO block is found, it's parsed using an `AVCInfoBlock` object, and the extracted text is set as the plug's name (`plug.setName()`).
        -   **(Potentially) Parse Other Plug-Related Descriptors:**
            -   May read and parse "Cluster EID Info Descriptor" if plugs are grouped.
            -   May read and parse "Audio Signal Mode Info Descriptor" for further details on signal characteristics.
        -   Returns `std::expected<void, IOKitError>` to indicate success or failure.

-   **MIDI Plug Detail Parsing (`parseMIDIPlugDetails` - Conceptual):**
    -   While not explicitly shown in full, a similar method `parseMIDIPlugDetails(MidiPlug& plug, uint8_t subunitID)` would exist if the library supports detailed MIDI plug parsing.
    -   It would read "MIDI Port Descriptors" and associated INFO blocks for names.
    -   It would populate a `MidiPlug` object (or similar structure) with information like the number of MIDI jacks, element ID, etc.

-   **Private Helper Methods:**
    -   The class likely contains private helper methods to parse individual descriptor types (e.g., a method to parse a single raw stream format descriptor into an `AudioStreamFormat` object, or to parse a raw audio port descriptor).
    -   These helpers would use `DescriptorUtils` for common field extraction.

**Overall Role:**
The `PlugDetailParser` is a specialized parser that focuses on the rich details of individual audio and MIDI plugs. After initial plug discovery (which might only yield plug IDs and basic types, e.g., by `UnitPlugDiscoverer`), the `DeviceParser` would likely use an instance of `PlugDetailParser` to:
1.  Flesh out `AudioPlug` (and potentially `MidiPlug`) objects with their full list of supported stream formats.
2.  Obtain human-readable names for these plugs from INFO block descriptors.
3.  Extract any other detailed characteristics defined in plug-specific or related descriptors.
This detailed information is crucial for applications to correctly configure and use the device's I/O capabilities.
