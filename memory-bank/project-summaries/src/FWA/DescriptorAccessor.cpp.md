# Summary for src/FWA/DescriptorAccessor.cpp

This C++ file implements the `FWA::DescriptorAccessor` class. Its primary responsibility is to read various types of descriptors from a FireWire audio device's configuration ROM using AV/C (Audio Video Control) commands. Descriptors contain vital information about the device's structure, capabilities, subunits, plugs, and supported audio formats.

**Key Functionalities:**

-   **Constructor (`DescriptorAccessor::DescriptorAccessor`):**
    -   Takes a `std::shared_ptr<CommandInterface>` (which is used to send AV/C commands) and an `std::shared_ptr<spdlog::logger>`.
    -   Stores these for later use.

-   **Core Descriptor Reading (`readDescriptor`):**
    -   `std::expected<std::vector<uint8_t>, IOKitError> readDescriptor(const DescriptorSpecifier& specifier, SubunitType subunitType = SubunitType::Unit, uint8_t subunitID = 0)`:
        -   This is the central method for reading an arbitrary descriptor.
        -   **Parameters:**
            -   `specifier`: A `DescriptorSpecifier` object that contains:
                -   `descriptor_type`: The type of descriptor to read (e.g., unit, subunit, audio plug, stream format).
                -   `descriptor_id`: An optional ID for specific descriptor instances.
                -   `address`: The starting address in the descriptor space (often 0 for top-level or list-based access).
                -   `length`: The number of bytes to read.
            -   `subunitType`, `subunitID`: The target subunit for the command (defaults to the main unit).
        -   **AV/C Command Construction:** It constructs an AV/C "READ DESCRIPTOR" command (opcode 0x02). The command payload includes:
            -   `descriptor_type_and_id`: Combined from `specifier.descriptor_type` and `specifier.descriptor_id`.
            -   `descriptor_specifier_length`: Set to 5 (for address and length fields).
            -   `descriptor_address`: From `specifier.address`.
            -   `descriptor_length`: From `specifier.length`.
        -   **Command Sending:** Uses `_commandInterface->sendCommand()` to send the constructed command to the specified subunit.
        -   **Response Parsing:**
            -   Validates the AV/C response (e.g., checks for "IMPLEMENTED/STABLE" or "ACCEPTED").
            -   Extracts the actual descriptor data from the response payload.
            -   Performs checks on the returned `descriptor_length` against the requested length.
        -   Returns a `std::expected` containing either the `std::vector<uint8_t>` of descriptor data on success or an `IOKitError` on failure.

-   **List-Based Descriptor Reading (`readDescriptorList`):**
    -   `std::expected<std::vector<std::vector<uint8_t>>, IOKitError> readDescriptorList(const DescriptorSpecifier& listSpecifier, SubunitType subunitType = SubunitType::Unit, uint8_t subunitID = 0)`:
        -   Reads a list of descriptors (e.g., a list of all audio plug descriptors).
        -   First, it reads a primary descriptor (specified by `listSpecifier`) that contains a `list_id` and `number_of_entries`.
        -   Then, it iterates `number_of_entries` times, constructing a new `DescriptorSpecifier` for each entry using the `list_id` and the current `entry_index`.
        -   It calls `readDescriptor()` for each entry and collects the results.

-   **Convenience Methods for Specific Descriptor Types:**
    -   The class provides several public methods tailored to read specific, commonly used descriptors:
        -   `readUnitInfo()`, `readSubunitInfo(SubunitType type, uint8_t id)`
        -   `readAudioPlugInfoList(uint8_t subunitID)`, `readMusicPlugInfoList(uint8_t subunitID)`
        -   `readStreamFormatInfoList(uint8_t subunitID, uint8_t plugID, bool isInputPlug)`
        -   `readJackDescriptionList(uint8_t subunitID, uint8_t plugID)`
        -   `readClusterEIDInfo(uint8_t subunitID, uint8_t plugID)`
        -   `readAudioSignalModeInfo(uint8_t subunitID, uint8_t plugID)`
        -   `readSamplingRateConverterInfo(uint8_t subunitID)`
        -   `readEffectControlInfoList(uint8_t subunitID)`
    -   These methods internally create the appropriate `DescriptorSpecifier` objects and call either `readDescriptor()` or `readDescriptorList()`.

**Overall Role:**
The `DescriptorAccessor` is a fundamental utility within the FWA library. It abstracts the low-level details of constructing and sending AV/C "READ DESCRIPTOR" commands and parsing their responses. It provides a clean interface for other components (like `DeviceParser` and specific subunit classes) to retrieve raw descriptor data from a FireWire device. This raw data is then further processed by those components to understand the device's capabilities and structure.
