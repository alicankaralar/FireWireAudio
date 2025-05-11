# Summary for include/FWA/DescriptorReader.hpp

This C++ header file defines the `FWA::DescriptorReader` class. This class is responsible for taking raw byte data, which represents various AV/C (Audio Video Control) descriptors (obtained via `DescriptorAccessor`), and parsing this data into more structured and usable C++ objects like `AudioPlug`, `AudioStreamFormat`, `AVCInfoBlock`, etc.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<vector>`: For `std::vector`.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<expected>`: For `std::expected` error handling.
    -   `<optional>`: For `std::optional`.
    -   `"DescriptorAccessor.hpp"`: For the `DescriptorAccessor` class, used to fetch raw descriptor data.
    -   `"DescriptorUtils.hpp"`: For utility functions to parse low-level fields from raw descriptor data.
    -   `"AudioPlug.hpp"`: For the `AudioPlug` class.
    -   `"AudioStreamFormat.hpp"`: For the `AudioStreamFormat` struct.
    -   `"AVCInfoBlock.hpp"`: For the `AVCInfoBlock` class.
    -   `"MusicSubunitCapabilities.hpp"`: For the `MusicSubunitCapabilities` class.
    -   `"Enums.hpp"`: For enums like `SubunitType`, `PlugDirection`.
    -   `"Error.h"`: For `IOKitError`.
    -   `<spdlog/spdlog.h>`: For logging.
    -   (Potentially other headers for specific struct/class definitions that parsed descriptors map to, e.g., a `UnitInfo` struct).

-   **Class `DescriptorReader`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `DescriptorReader(std::shared_ptr<DescriptorAccessor> accessor, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the reader with a `DescriptorAccessor` (to fetch raw data if needed for list-based descriptors) and a logger.
        -   **Direct Parsing Methods (from raw data):**
            -   These methods take a `std::vector<uint8_t>` containing the raw data of a single descriptor and parse it into a specific C++ object. They heavily rely on `DescriptorUtils` for extracting fields.
            -   `std::expected<ParsedUnitInfo, IOKitError> parseUnitInfoDescriptor(const std::vector<uint8_t>& data) const;` (Assuming `ParsedUnitInfo` is a struct to hold unit-level info like vendor/model ID).
            -   `std::expected<ParsedSubunitInfo, IOKitError> parseSubunitInfoDescriptor(const std::vector<uint8_t>& data) const;` (Assuming `ParsedSubunitInfo` for subunit type/id).
            -   `std::expected<AudioPlug, IOKitError> parseAudioPlugInfoDescriptor(const std::vector<uint8_t>& data, PlugDirection direction) const;`
            -   `std::expected<AudioStreamFormat, IOKitError> parseStreamFormatDescriptor(const std::vector<uint8_t>& data) const;`
            -   `std::expected<AVCInfoBlock, IOKitError> parseInfoBlockDescriptor(const std::vector<uint8_t>& data) const;`
            -   `std::expected<MusicSubunitCapabilities, IOKitError> parseMusicSubunitCapabilitiesDescriptor(const std::vector<uint8_t>& data) const;`
            -   (And others for different descriptor types like MIDI plug descriptors, audio port descriptors, etc.)
        -   **List Reading and Parsing Methods:**
            -   These methods are more high-level. They use the `_accessor` to read a "list descriptor" (which indicates the number of items in the list and potentially the type of items). Then, they iterate, using the `_accessor` again to read each individual item descriptor from the list, and then call the appropriate direct parsing method (above) for each item.
            -   `std::expected<std::vector<AudioPlug>, IOKitError> getAudioPlugs(uint8_t subunitID, PlugDirection direction) const;`
            -   `std::expected<std::vector<AudioStreamFormat>, IOKitError> getStreamFormats(uint8_t subunitID, uint8_t plugID, bool isSourcePlug) const;`
            -   `std::expected<std::vector<SubunitIdentifier>, IOKitError> getSubunitIdentifiers() const;` (Reads the unit's subunit table).

    -   **Private Members (Conceptual - implementation details in .cpp):**
        -   `_accessor`: `std::shared_ptr<DescriptorAccessor>`.
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.

**Overall Role:**
The `DescriptorReader` class acts as an intermediary layer between raw descriptor byte data (obtained by `DescriptorAccessor`) and structured C++ objects that represent the device's features and capabilities. It encapsulates the logic for interpreting the byte layout of various AV/C descriptors and transforming them into meaningful data structures.
This class is a key component used by:
-   `DeviceParser`: To parse all the descriptors of a device and build up the comprehensive `DeviceInfo` object.
-   Specific subunit classes (like `AudioSubunit`, `MusicSubunit`): During their `parseDescriptors` methods to understand their own specific configurations.
By separating the concerns of fetching raw data (`DescriptorAccessor`), parsing raw data into fields (`DescriptorUtils`), and assembling those fields into higher-level objects (`DescriptorReader`), the FWA library achieves a more modular and maintainable design for device discovery and capability assessment.
