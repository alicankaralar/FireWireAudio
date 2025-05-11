# Summary for include/FWA/DeviceParser.hpp

This C++ header file defines the `FWA::DeviceParser` class. The primary responsibility of this class is to orchestrate the comprehensive parsing of a FireWire audio device's AV/C (Audio Video Control) descriptors. It uses a `DescriptorReader` to obtain structured C++ objects from raw descriptor data and populates a `DeviceInfo` object with all the gathered information.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<memory>`: For `std::shared_ptr`.
    -   `<vector>`: For `std::vector`.
    -   `<expected>`: For `std::expected`.
    -   `"DeviceInfo.hpp"`: For the `DeviceInfo` struct that this parser populates.
    -   `"DescriptorReader.hpp"`: For the `DescriptorReader` class, which is used to parse raw descriptor data into C++ objects.
    -   `"Subunit.hpp"`: For `SubunitIdentifier` (likely a struct or typedef for `FWA::Subunit::Identifier`).
    -   `"CommandInterface.h"`: For `CommandInterface` (though primarily used by the `DescriptorReader`'s `DescriptorAccessor`).
    -   `"Error.h"`: For `IOKitError`.
    -   `<spdlog/spdlog.h>`: For logging.

-   **Class `DeviceParser`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `DeviceParser(std::shared_ptr<DescriptorReader> reader, std::shared_ptr<CommandInterface> commandInterface, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the parser with a `DescriptorReader` (its main tool for getting parsed descriptor objects), a `CommandInterface` (which the `DescriptorReader`'s underlying `DescriptorAccessor` uses), and a logger.
        -   **Main Parsing Method:**
            -   `std::expected<DeviceInfo, IOKitError> parse(const std::vector<SubunitIdentifier>& subunitIdentifiers);`
            -   This is the primary public method to start the parsing process for a device.
            -   **Parameter:** `subunitIdentifiers` - A vector of `SubunitIdentifier` objects, previously discovered by `SubunitDiscoverer`, indicating which subunits are present on the device.
            -   **Operation (Conceptual - details in .cpp):**
                1.  Creates an empty `DeviceInfo` object.
                2.  **Parse Unit-Level Information:** Calls a private helper method (e.g., `parseUnitInformation`) that uses the `_reader` to:
                    -   Read and parse the "Unit Information" descriptor to get vendor ID, model ID.
                    -   Read and parse associated INFO blocks for vendor name, model name, serial number, firmware version.
                    -   Discover and parse unit-level plugs using `_reader->getAudioPlugs(kUnitSubunitID, ...)` and populates `deviceInfo.inputPlugs` and `deviceInfo.outputPlugs`.
                3.  **Parse Subunit-Level Information:** Iterates through the provided `subunitIdentifiers`:
                    -   If an Audio Subunit is found, calls a private helper (e.g., `parseAudioSubunitDetails`) that uses `_reader` to:
                        -   Parse audio subunit specific descriptors.
                        -   Discover and parse audio plugs specific to this subunit (if any, beyond unit-level plugs) and add them to `deviceInfo.inputPlugs`/`outputPlugs` or a nested structure in `deviceInfo.audioSubunitInfo`.
                        -   Populate `deviceInfo.audioSubunitInfo`.
                    -   If a Music Subunit is found, calls a private helper (e.g., `parseMusicSubunitDetails`) that uses `_reader` to:
                        -   Parse `MusicSubunitCapabilitiesDescriptor`.
                        -   Discover and parse MIDI plugs.
                        -   Populate `deviceInfo.musicSubunitInfo`.
                4.  Returns the fully populated `DeviceInfo` object wrapped in `std::expected`, or an `IOKitError` if any critical parsing step fails.

    -   **Private Members (Conceptual - implementation details in .cpp):**
        -   `_reader`: `std::shared_ptr<DescriptorReader>`.
        -   `_commandInterface`: `std::shared_ptr<CommandInterface>` (primarily for the `_reader`).
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.
        -   Private helper methods for parsing specific parts of the device structure (e.g., `parseUnitInformation`, `parseAudioSubunitDetails`, `parseMusicSubunitDetails`).

**Overall Role:**
The `DeviceParser` class acts as the main orchestrator for understanding the complete structure and capabilities of a FireWire audio device. It takes the initial list of discovered subunits and then systematically uses the `DescriptorReader` to:
1.  Fetch and parse unit-level information (names, versions, unit plugs).
2.  Fetch and parse information for each specific subunit (Audio, Music), including their plugs and capabilities.
The result of its `parse()` method is a comprehensive `DeviceInfo` object that serves as the FWA library's model of the device. This class is a key component used by `AudioDevice::init()` to build its internal representation of the hardware.
