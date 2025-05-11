# Summary for src/FWA/DescriptorUtils.cpp

This C++ file implements a collection of utility functions within the `FWA::DescriptorUtils` namespace. These functions are designed to help parse common fields and structures found within AV/C (Audio Video Control) descriptors read from FireWire devices.

**Key Functionalities:**

-   **Parsing Standard AV/C Identifiers:**
    -   `SubunitIdentifier parseSubunitIdentifier(const std::vector<uint8_t>& data, size_t& offset)`:
        -   Parses a 5-byte subunit identifier from the provided `data` vector, starting at the given `offset`.
        -   Extracts the `subunit_type` (e.g., audio, music, unit) and `subunit_ID` (0-7).
        -   Updates the `offset` to point past the parsed bytes.
        -   Returns a `SubunitIdentifier` struct (likely defined in a header).
    -   `uint8_t parsePlugIdentifier(const std::vector<uint8_t>& data, size_t& offset)`:
        -   Parses a 1-byte plug identifier (plug ID) and updates the offset.
    -   `ExtendedSubunitIdentifier parseExtendedSubunitIdentifier(const std::vector<uint8_t>& data, size_t& offset)`:
        -   Parses an extended subunit identifier, which includes `subunit_type`, `subunit_ID`, and an `extension_code`.

-   **Stream Format Parsing:**
    -   `StreamFormatInfo parseStreamFormat(const std::vector<uint8_t>& data, size_t& offset)`:
        -   Parses a stream format code, typically found in descriptors like `stream_format_descriptor` or `audio_terminal_descriptor`.
        -   It interprets the format code byte(s) to determine the `StreamFormatType` (e.g., AM824 Non-blocked Audio, DVCR).
        -   For some formats (like AM824), it can also extract the number of channels directly from the format code.
        -   Returns a `StreamFormatInfo` struct containing the type and channel count.

-   **Audio Signal Format Parsing:**
    -   `AudioSignalFormat parseAudioSignalFormat(const std::vector<uint8_t>& data, size_t& offset)`:
        -   Parses an audio signal format descriptor. This descriptor provides more detailed information about an audio stream, such as:
            -   Sampling frequency (SFC).
            -   Bit depth or quantization.
            -   Other audio characteristics.
        -   The parsed information is returned in an `AudioSignalFormat` struct.

-   **SFC (Sampling Frequency Code) Conversion:**
    -   `double sfcToSampleRate(uint8_t sfc)`:
        -   Converts an SFC byte (as found in CIP headers or some descriptors) into a `double` representing the actual sample rate in Hz (e.g., `0x00` -> 32000.0 Hz, `0x01` -> 44100.0 Hz, `0x02` -> 48000.0 Hz).
    -   `uint8_t sampleRateToSFC(double sampleRate)`:
        -   Performs the reverse conversion, from a sample rate `double` to an SFC byte.

-   **String Conversion Utilities:**
    -   `std::string bytesToHexString(const std::vector<uint8_t>& bytes)`:
        -   Converts a vector of bytes into a hexadecimal string representation (e.g., for logging or display).
    -   `std::string trimString(const std::string& str)`:
        -   Removes leading and trailing whitespace characters from a string. This is useful when parsing textual data from descriptors that might have extra padding.

**Overall Role:**
The `DescriptorUtils` namespace provides a set of low-level helper functions that are essential for the higher-level descriptor parsing logic within the FWA library. Classes like `DeviceParser`, `AudioSubunit`, and `MusicSubunitDescriptorParser` would use these utilities to decode the raw byte arrays (obtained via `DescriptorReader` and `DescriptorAccessor`) into meaningful data structures and values representing the device's capabilities. These utilities promote code reuse and consistency in interpreting standard AV/C descriptor fields.
