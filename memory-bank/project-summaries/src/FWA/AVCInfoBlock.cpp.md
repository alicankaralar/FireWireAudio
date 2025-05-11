# Summary for src/FWA/AVCInfoBlock.cpp

This C++ file implements the `FWA::AVCInfoBlock` class. This class is designed to parse and represent an "INFO block" descriptor, which is a type of descriptor found in the AV/C (Audio Video Control) specification. INFO blocks are used to provide textual information, often associated with other descriptors or elements within a subunit (like names for plugs, controls, or stream formats).

**Key Functionalities:**

-   **Constructor:**
    -   Default constructor initializes member variables.

-   **Parsing (`parse(const std::vector<uint8_t>& data)`):**
    -   This is the primary method of the class. It takes a `std::vector<uint8_t>` containing the raw byte data of an INFO block descriptor.
    -   **Validation:** Checks if the provided data is large enough to contain the basic INFO block structure.
    -   **Field Extraction:** It parses various fields from the INFO block data according to the AV/C specification:
        -   `info_block_type` (should be 0x07 for an INFO block).
        -   `is_list_index_valid`: A boolean flag indicating if the `list_index` field is valid.
        -   `list_index`: If valid, an index that might refer to an item in a list described elsewhere.
        -   `constant_field`: A field that can categorize the type of information (e.g., model name, vendor name, plug name). The meaning of this constant is context-dependent.
        -   `character_set`: An enum value indicating the character encoding of the textual data (e.g., ASCII, ISO Latin 1, UCS-2, UTF-8, UTF-16BE).
        -   `language_code`: (Often not used or set to a default).
        -   `text_length`: The length of the actual textual data.
        -   `textual_data`: The raw bytes of the text.
    -   **Text Conversion:**
        -   Based on the `character_set` field, it attempts to convert the `textual_data` into a UTF-8 encoded `std::string`.
        -   It handles ASCII directly. For other encodings like UCS-2 or UTF-16BE, it would perform the necessary byte swapping and conversion to UTF-8. (The snippet shows placeholders for more complex conversions).
    -   The parsed and converted data is stored in member variables.

-   **Data Storage (Member Variables):**
    -   `_isListIndexValid`
    -   `_listIndex`
    -   `_constantField`
    -   `_characterSet`
    -   `_textualData` (as a `std::string`, converted to UTF-8)

-   **Accessor Methods (Getters):**
    -   `isListIndexValid() const`
    -   `getListIndex() const`
    -   `getConstantField() const`
    -   `getCharacterSet() const` (returns the original character set enum)
    -   `getTextualData() const` (returns the UTF-8 `std::string`)

-   **JSON Serialization (`toJson() const`):**
    -   Converts the parsed INFO block information into a `nlohmann::json` object.
    -   This typically includes the `constant_field` and the `textual_data`.

**Overall Role:**
The `AVCInfoBlock` class is a utility used during the device descriptor parsing process (likely by `DeviceParser` or specific subunit parsers). When an INFO block descriptor is encountered in the device's configuration ROM, an instance of `AVCInfoBlock` can be used to parse its contents. This allows the FWA library to extract human-readable strings (like names for various device elements) and other categorical information provided by the device manufacturer.
