# Summary for include/FWA/MusicSubunitDescriptorParser.hpp

This C++ header file defines the `FWA::MusicSubunitDescriptorParser` class. This class is specifically designed to parse the descriptors associated with an AV/C (Audio Video Control) Music Subunit on a FireWire device, with the primary goal of extracting its MIDI (Musical Instrument Digital Interface) capabilities.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<memory>`: For `std::shared_ptr`.
    -   `<expected>`: For `std::expected` error handling.
    -   `"DescriptorAccessor.hpp"`: For the `DescriptorAccessor` class, which is used to fetch raw descriptor data from the device.
    -   `"MusicSubunitCapabilities.hpp"`: For the `MusicSubunitCapabilities` class, which will store the parsed capabilities.
    -   `"Error.h"`: For `IOKitError`.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<cstdint>`: For `uint8_t`.

-   **Class `MusicSubunitDescriptorParser`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `MusicSubunitDescriptorParser(std::shared_ptr<DescriptorAccessor> accessor, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the parser with a `DescriptorAccessor` (to read raw descriptor data) and a logger.
        -   **Main Parsing Method:**
            -   `std::expected<MusicSubunitCapabilities, IOKitError> parse(uint8_t subunitID) const;`
            -   This is the primary public method to initiate parsing for a given Music Subunit.
            -   **Parameter:** `subunitID` - The ID of the Music Subunit whose capabilities are to be parsed.
            -   **Operation (Conceptual - details in .cpp):**
                1.  Uses the `_accessor` (likely calling `_accessor->readSubunitInfo(SubunitType::Music, subunitID)` or a more specific method if available) to fetch the raw byte data of the "Music Subunit Capabilities Descriptor". This descriptor is the main source of information about the subunit's MIDI features.
                2.  Creates an instance of `MusicSubunitCapabilities`.
                3.  If the raw descriptor data is successfully read, it calls the `setCapabilitiesFromData()` method of the `MusicSubunitCapabilities` object, passing the raw data. The `setCapabilitiesFromData()` method (implemented in `MusicSubunitCapabilities.cpp`) then decodes the bitfields within the raw data to populate the boolean flags and plug counts in the `MusicSubunitCapabilities` object.
                4.  Returns an `std::expected` containing the populated `MusicSubunitCapabilities` object on success, or an `IOKitError` if reading or parsing fails.
            -   **(Note:** While this parser focuses on the capabilities descriptor, a more comprehensive parsing of a Music Subunit by `DeviceParser` or `MusicSubunit::parseDescriptors` might also involve discovering MIDI plugs and their names using other descriptors, potentially via `PlugDetailParser` or `DescriptorReader`.)

    -   **Private Members:**
        -   `std::shared_ptr<DescriptorAccessor> _accessor;`: The accessor used to read raw descriptor data.
        -   `std::shared_ptr<spdlog::logger> _logger;`: The logger instance.

**Overall Role:**
The `MusicSubunitDescriptorParser` is a specialized utility class within the FWA library. Its specific task is to:
1.  Retrieve the "Music Subunit Capabilities Descriptor" for a given Music Subunit using a `DescriptorAccessor`.
2.  Instantiate and populate a `MusicSubunitCapabilities` object from this raw descriptor data.
This provides a structured representation of the MIDI features of the Music Subunit, which is then used by the `MusicSubunit` class and ultimately contributes to the overall `DeviceInfo`. It helps in understanding what MIDI functionalities (like number of IN/OUT ports, SysEx support, etc.) the device offers.
