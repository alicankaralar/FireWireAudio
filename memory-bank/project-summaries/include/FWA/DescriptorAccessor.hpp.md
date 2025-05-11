# Summary for include/FWA/DescriptorAccessor.hpp

This C++ header file defines the `FWA::DescriptorAccessor` class. The primary purpose of this class is to provide an interface for reading raw AV/C (Audio Video Control) descriptor data from a FireWire device. It uses an underlying `CommandInterface` to send the necessary "READ_DESCRIPTOR" AV/C commands.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<vector>`: For `std::vector<uint8_t>` to hold raw descriptor data.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<expected>`: For `std::expected` error handling.
    -   `"CommandInterface.h"`: For the `CommandInterface` class, which is used to send AV/C commands.
    -   `"DescriptorSpecifier.hpp"`: For the `DescriptorSpecifier` struct, which is used to specify which descriptor to read.
    -   `"Enums.hpp"`: For `SubunitType`.
    -   `"Error.h"`: For `IOKitError`.
    -   `<spdlog/spdlog.h>`: For logging.

-   **Class `DescriptorAccessor`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `DescriptorAccessor(std::shared_ptr<CommandInterface> commandInterface, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the accessor with a shared pointer to a `CommandInterface` (which it will use to send commands) and a logger.
        -   **Core Descriptor Reading Method:**
            -   `std::expected<std::vector<uint8_t>, IOKitError> readDescriptor(const DescriptorSpecifier& specifier, uint16_t length = 0) const;`
            -   This is the fundamental method for reading any descriptor.
            -   **Parameters:**
                -   `specifier`: A `DescriptorSpecifier` object that contains all the necessary information to target a specific descriptor (e.g., subunit type, subunit ID, descriptor type, descriptor ID, list ID, entry position).
                -   `length`: An optional parameter specifying the number of bytes to read from the descriptor. If 0 (or a default value), it might first read a small part of the descriptor to determine its actual length, then read the full descriptor.
            -   **Implementation (in .cpp):**
                -   Constructs the AV/C "READ_DESCRIPTOR" command payload. This payload includes:
                    -   The `descriptor_specifier` field (copied from the `specifier` parameter).
                    -   The `length` field.
                -   Uses `_commandInterface->sendCommand()` to send this "READ_DESCRIPTOR" command to the appropriate subunit (derived from `specifier.subunit_type` and `specifier.subunit_id`).
                -   The AV/C response to "READ_DESCRIPTOR" contains the raw descriptor data in its payload.
                -   Returns this raw descriptor data as a `std::vector<uint8_t>` wrapped in `std::expected`, or an `IOKitError` if the command fails or the device returns an error.
        -   **Convenience Read Methods (Wrappers around `readDescriptor`):**
            -   The header declares several convenience methods for reading common types of descriptors. These methods internally create the appropriate `DescriptorSpecifier` and call the core `readDescriptor` method.
            -   `std::expected<std::vector<uint8_t>, IOKitError> readUnitInfo(uint8_t unitSubunitID = 0x1F) const;` (Reads the Unit Information descriptor).
            -   `std::expected<std::vector<uint8_t>, IOKitError> readSubunitInfo(SubunitType type, uint8_t subunitID) const;` (Reads a Subunit Information descriptor).
            -   `std::expected<std::vector<std::vector<uint8_t>>, IOKitError> readAudioPlugInfoList(uint8_t subunitID, bool isInput) const;` (Reads a list of Audio Plug Info descriptors).
            -   `std::expected<std::vector<std::vector<uint8_t>>, IOKitError> readStreamFormatInfoList(uint8_t subunitID, uint8_t plugID, bool isSourcePlug) const;` (Reads a list of Stream Format Info descriptors for a specific plug).
            -   `std::expected<std::vector<uint8_t>, IOKitError> readInfoBlock(uint8_t listID, uint16_t entryPosition) const;` (Reads a specific INFO block).
            -   (And potentially others for MIDI plugs, audio port descriptors, etc.)

    -   **Private Members (Conceptual - implementation details in .cpp):**
        -   `_commandInterface`: `std::shared_ptr<CommandInterface>`.
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.

**Overall Role:**
The `DescriptorAccessor` class provides a crucial service within the FWA library by abstracting the low-level details of fetching raw AV/C descriptor data from a FireWire device. It translates requests for specific descriptors (identified by `DescriptorSpecifier` or through convenience methods) into the appropriate "READ_DESCRIPTOR" AV/C commands, sends them via the `CommandInterface`, and returns the raw byte data of the descriptor.
This class is heavily used by:
-   `DescriptorReader`: Which uses `DescriptorAccessor` to get raw data and then parses it into more structured C++ objects (like `AudioPlug`, `AudioStreamFormat`, `AVCInfoBlock`).
-   Various parser classes (like `DeviceParser`, `PlugDetailParser`, `SubunitDiscoverer`) that need to read specific types of descriptors to understand the device's capabilities.
