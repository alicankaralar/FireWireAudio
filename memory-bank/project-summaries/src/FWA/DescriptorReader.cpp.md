# Summary for src/FWA/DescriptorReader.cpp

This C++ file implements the `FWA::DescriptorReader` class. The primary purpose of this class is to read raw descriptor data from a FireWire device's configuration ROM. It achieves this by constructing and sending AV/C "READ DESCRIPTOR" commands using a provided `CommandInterface`.

**Key Functionalities:**

-   **Constructor (`DescriptorReader::DescriptorReader`):**
    -   Takes a `std::shared_ptr<CommandInterface>` and an `std::shared_ptr<spdlog::logger>`.
    -   Stores these for use in its methods. The `CommandInterface` is essential for actually sending commands to the FireWire device.

-   **Core Read Method (`read`):**
    -   `std::expected<std::vector<uint8_t>, IOKitError> read(uint32_t address, uint16_t length, SubunitType subunitType = SubunitType::Unit, uint8_t subunitID = 0) const`:
        -   This is the fundamental operation for reading a block of data from the device's descriptor space.
        -   **Parameters:**
            -   `address`: The starting address within the descriptor space to read from.
            -   `length`: The number of bytes to read.
            -   `subunitType`: The type of the target subunit (e.g., Unit, Audio, Music). Defaults to `Unit`.
            -   `subunitID`: The ID of the target subunit. Defaults to 0.
        -   **AV/C Command Construction:**
            -   It assembles an AV/C "READ DESCRIPTOR" command (opcode `0x02`).
            -   The command payload includes:
                -   `descriptor_type_and_id`: Set to `0xFF` (any type) and `0xFF` (any ID) because this method reads a raw block by address and length, not a specific typed descriptor by ID.
                -   `descriptor_specifier_length`: Set to 5 (for the 4-byte address and 1-byte length field that follows in *this specific command's interpretation*, though the AV/C spec for READ DESCRIPTOR uses a 2-byte length in the request).
                -   `descriptor_address`: The `address` parameter.
                -   `descriptor_length`: The `length` parameter.
        -   **Command Sending:**
            -   Uses `_commandInterface->sendCommand()` to send the constructed command to the specified subunit.
        -   **Response Handling:**
            -   Checks the AV/C response status. If not "IMPLEMENTED/STABLE" or "ACCEPTED", it returns an error.
            -   Validates that the response payload is large enough to contain the returned descriptor length and the actual data.
            -   Extracts the `returned_descriptor_length` from the response.
            -   If `returned_descriptor_length` does not match the requested `length`, it logs a warning but proceeds to copy the `returned_descriptor_length` amount of data. This handles cases where a device might return less data than requested if the request goes past the end of a descriptor.
            -   Copies the descriptor data from the response payload into a `std::vector<uint8_t>`.
        -   Returns a `std::expected` containing the vector of raw descriptor bytes or an `IOKitError`.

-   **Convenience Method (`readDescriptor`):**
    -   `std::expected<std::vector<uint8_t>, IOKitError> readDescriptor(const DescriptorSpecifier& specifier, SubunitType subunitType, uint8_t subunitID) const`:
        -   This method is designed to be used by `DescriptorAccessor`.
        -   It takes a `DescriptorSpecifier` object (which contains the descriptor type, ID, address, and length).
        -   It directly calls the core `read()` method using the `address` and `length` from the `specifier`.
        -   The `descriptor_type` and `descriptor_id` from the `specifier` are used in the AV/C command payload construction within the `read()` method (though the `read()` method itself sets them to `0xFF` for raw block reads, this `readDescriptor` method in `DescriptorReader` is a more direct passthrough of address and length).

**Overall Role:**
The `DescriptorReader` class provides a foundational service for fetching raw descriptor data from a FireWire device. It encapsulates the logic for forming and sending the AV/C "READ DESCRIPTOR" command and handling its response. This raw data is then typically passed to higher-level parsing classes (like `DescriptorAccessor`, `DeviceParser`, or specific descriptor type parsers) to interpret its meaning and extract structured information about the device's capabilities.
