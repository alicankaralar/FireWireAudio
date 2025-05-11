# Summary for include/FWA/DescriptorUtils.hpp

This C++ header file defines the `FWA::DescriptorUtils` namespace. This namespace contains a collection of static utility functions designed to parse low-level fields from raw AV/C (Audio Video Control) descriptor byte data. These functions are essential for interpreting the binary data read from a FireWire device's configuration ROM or descriptor space.

**Key Declarations and Components:**

-   **Namespace `FWA::DescriptorUtils`:** All utility functions are grouped within this namespace.

-   **Includes:**
    -   `<vector>`: For `std::vector<uint8_t>`.
    -   `<string>`: For `std::string`.
    -   `<cstdint>`: For fixed-width integer types like `uint8_t`, `uint16_t`, `uint32_t`.
    -   `"Enums.hpp"`: For enums like `SubunitType`, `DescriptorType`, `PlugDirection`.
    -   `"AudioStreamFormat.hpp"`: For the `AudioStreamFormat` struct.
    -   (Potentially `"Helpers.hpp"` if byte swapping utilities are used from there, though direct bit manipulation might also occur here).

-   **Static Utility Functions:**
    -   Most functions take `const std::vector<uint8_t>& data` (the raw byte buffer of a descriptor or part of it) and `size_t& offset` (a reference to the current read offset within the data, which is advanced by the function after reading).
    -   **Basic Type Parsers:**
        -   `uint8_t parseUInt8(const std::vector<uint8_t>& data, size_t& offset);`
        -   `uint16_t parseUInt16(const std::vector<uint8_t>& data, size_t& offset);` (Handles big-endian to host-endian conversion).
        -   `uint32_t parseUInt32(const std::vector<uint8_t>& data, size_t& offset);` (Handles big-endian to host-endian conversion).
    -   **Descriptor-Specific Field Parsers:**
        -   `SubunitIdentifier parseSubunitIdentifier(const std::vector<uint8_t>& data, size_t& offset);`: Parses the `subunit_type` and `subunit_id` fields (typically 1 byte) and returns a `SubunitIdentifier` struct (which would contain these two fields).
        -   `uint8_t parsePlugID(const std::vector<uint8_t>& data, size_t& offset);`: Parses a plug ID.
        -   `uint8_t parseChannelCount(const std::vector<uint8_t>& data, size_t& offset);`: Parses a channel count field.
        -   `AudioStreamFormat parseAudioSignalFormat(const std::vector<uint8_t>& data, size_t& offset);`: Parses fields related to an audio signal format from a Stream Format Descriptor, such as Sample Frequency Code (SFC), bit depth code, number of channels, and potentially other flags, then constructs and returns an `AudioStreamFormat` object.
        -   `uint8_t parseSampleRateCode(const std::vector<uint8_t>& data, size_t& offset);`: Parses an SFC (Sample Frequency Code) and might convert it to an actual sample rate or an enum value.
        -   `uint8_t parseBitDepthCode(const std::vector<uint8_t>& data, size_t& offset);`: Parses a bit depth code and might convert it to actual bits.
        -   `std::string parseString(const std::vector<uint8_t>& data, size_t& offset, size_t length, uint16_t charSet);`: Parses a string of a given `length` from the data, considering the specified `charSet` (character set ID). This would involve converting the raw bytes to a `std::string`, potentially handling character encodings like UTF-16BE (common in AV/C) to UTF-8.
    -   **List Descriptor Field Parsers:**
        -   `uint16_t parseListLength(const std::vector<uint8_t>& data, size_t& offset);`: Parses the `descriptor_list_length` field from a list descriptor.
        -   `uint8_t parseEntryCount(const std::vector<uint8_t>& data, size_t& offset);`: Parses the `number_of_entries` field from a list descriptor.
    -   **Other Utility Functions:**
        -   Functions to extract specific bits or bitfields from a byte.
        -   Functions to convert AV/C specific codes (like SFC or bit depth codes) into more usable values (e.g., actual sample rate as a double, bit depth as a uint8_t).

**Overall Role:**
The `DescriptorUtils` namespace provides a collection of low-level, stateless utility functions that are essential for the process of parsing AV/C descriptors. They encapsulate the knowledge of the byte layout and bit-level structure of various descriptor fields as defined by the IEC 61883 and related AV/C specifications.
These functions are heavily used by:
-   `DescriptorReader`: To break down raw descriptor data (obtained from `DescriptorAccessor`) into meaningful primitive types or simple structs.
-   Higher-level parser classes (like `DeviceParser`, `PlugDetailParser`, `AudioSubunit`, `MusicSubunit`) indirectly through `DescriptorReader`, or directly if they need to parse very specific custom fields.
By centralizing these parsing utilities, the FWA library ensures consistency in how descriptor data is interpreted and reduces code duplication across different parser components. They form the foundation for understanding the capabilities and structure of a FireWire audio device.
