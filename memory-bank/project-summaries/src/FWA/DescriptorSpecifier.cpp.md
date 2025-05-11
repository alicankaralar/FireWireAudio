# Summary for src/FWA/DescriptorSpecifier.cpp

This C++ file implements the `FWA::DescriptorSpecifier` struct. This structure is a Plain Old Data (POD) type used to encapsulate the parameters required to identify and read a specific descriptor from a FireWire device's configuration ROM.

**Key Members (Fields):**

-   **`uint8_t descriptor_type`**:
    -   Indicates the type of descriptor to be read. The values for this field are typically defined by the AV/C (Audio Video Control) or IEC 61883 standards. Examples include codes for:
        -   Unit descriptors
        -   Subunit Information descriptors
        -   Audio Plug descriptors
        -   Music Plug descriptors
        -   Stream Format descriptors
        -   INFO block descriptors, etc.

-   **`uint8_t descriptor_id`**:
    -   An identifier that, in conjunction with `descriptor_type`, can specify a particular instance of a descriptor.
    -   For example, if `descriptor_type` indicates an "audio plug descriptor," `descriptor_id` might be the specific plug number (0, 1, 2, etc.).
    -   For some descriptor types, this field might be unused or set to a default (e.g., 0xFF for "any ID" if not applicable).

-   **`uint32_t address`**:
    -   Specifies the starting address within the device's descriptor space from which to begin reading.
    -   For descriptors that are part of a list (e.g., a list of stream formats for a plug), this field is often used as an `entry_index` (0-based) into that list, rather than a direct memory address. The actual memory address is then resolved by the device based on the `list_id` (from a parent descriptor) and this `entry_index`.
    -   For directly addressable descriptors, this would be the offset.

-   **`uint16_t length`**:
    -   Specifies the number of bytes to read for the descriptor.
    -   The caller usually needs to know the expected length of the descriptor type being requested, or a maximum possible length. The device will return up to this many bytes.

**Overall Role:**
The `DescriptorSpecifier` struct is a simple but crucial data container. It's used as a parameter to methods in classes like `DescriptorAccessor` and `DescriptorReader`. These classes use the information in a `DescriptorSpecifier` to:
1.  Construct the appropriate AV/C "READ DESCRIPTOR" command.
2.  Specify the target descriptor type, ID (if applicable), starting address or index, and the amount of data to fetch from the FireWire device.

It standardizes how requests for different descriptors are formulated within the FWA library.
