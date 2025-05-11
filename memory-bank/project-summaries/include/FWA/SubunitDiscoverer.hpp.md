# Summary for include/FWA/SubunitDiscoverer.hpp

This C++ header file defines the `FWA::SubunitDiscoverer` class. The purpose of this class is to discover the AV/C (Audio Video Control) subunits (such as Audio, Music, or Vendor-Unique subunits) that are present on a FireWire device. It achieves this by using a `DescriptorAccessor` to read and interpret the "Subunit Information" descriptors, which are typically found at the unit level of the device's descriptor tree.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<vector>`: For `std::vector`.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<optional>`: For `std::optional`.
    -   `<expected>`: For `std::expected` error handling.
    -   `"DescriptorAccessor.hpp"`: For the `DescriptorAccessor` class, used to read raw descriptor data.
    -   `"Subunit.hpp"`: For the `SubunitIdentifier` struct (which contains `SubunitType` and `id`) and the `SubunitType` enum.
    -   `"Error.h"`: For `IOKitError`.
    -   `<spdlog/spdlog.h>`: For logging.

-   **Class `SubunitDiscoverer`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `SubunitDiscoverer(std::shared_ptr<DescriptorAccessor> accessor, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the discoverer with a `DescriptorAccessor` (to read the raw subunit table descriptor) and a logger.
        -   **Primary Discovery Method:**
            -   `std::expected<std::vector<SubunitIdentifier>, IOKitError> discoverSubunits(std::optional<SubunitType> typeToDiscover = std::nullopt) const;`
            -   This is the main method for discovering subunits.
            -   **Parameter:** `typeToDiscover` (optional) - If a `SubunitType` is provided, the method will filter the results and only return subunits of that specific type. If `std::nullopt` (the default), it discovers all types of subunits listed by the device.
            -   **Operation (Conceptual - details in .cpp):**
                1.  Uses the `_accessor` to read the "Subunit Table" descriptor (or a list of "Subunit Identifier Descriptors") from the unit level of the device. This descriptor contains entries, each specifying a `subunit_type` and a `subunit_id`.
                2.  Iterates through the raw entries obtained from the accessor.
                3.  For each entry, it parses the `subunit_type` and `subunit_id` (likely using `DescriptorUtils::parseSubunitIdentifier`).
                4.  If `typeToDiscover` was specified, it checks if the parsed `subunit_type` matches the requested type.
                5.  If there's a match (or if no specific type was requested for filtering), it creates a `SubunitIdentifier` struct and adds it to a result vector.
                6.  Returns an `std::expected` containing this `std::vector<SubunitIdentifier>` on success, or an `IOKitError` if reading or parsing fails.
        -   **Convenience Discovery Methods:**
            -   `std::expected<std::vector<SubunitIdentifier>, IOKitError> discoverAudioSubunits() const;`
                -   A helper method that calls `discoverSubunits(SubunitType::Audio)` to specifically find Audio subunits.
            -   `std::expected<std::vector<SubunitIdentifier>, IOKitError> discoverMusicSubunits() const;`
                -   A helper method that calls `discoverSubunits(SubunitType::Music)` to specifically find Music (MIDI) subunits.

    -   **Private Members:**
        -   `std::shared_ptr<DescriptorAccessor> _accessor;`: The accessor used to read descriptor data.
        -   `std::shared_ptr<spdlog::logger> _logger;`: The logger instance.

**Overall Role:**
The `SubunitDiscoverer` class is a crucial first step in understanding the internal architecture of a FireWire audio device after basic connection. It is used by the `AudioDevice` class (typically during its `init()` method) to:
1.  Query the device for a list of all its functional subunits.
2.  Identify the type (e.g., Audio, Music) and ID of each subunit.
This information (`std::vector<SubunitIdentifier>`) is then used by `AudioDevice` to instantiate the appropriate derived `Subunit` objects (e.g., `AudioSubunit`, `MusicSubunit`), which can then proceed to parse their own more detailed descriptors.
