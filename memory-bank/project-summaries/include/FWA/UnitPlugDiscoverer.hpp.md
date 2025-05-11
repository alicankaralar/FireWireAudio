# Summary for include/FWA/UnitPlugDiscoverer.hpp

This C++ header file defines the `FWA::UnitPlugDiscoverer` class. The responsibility of this class is to discover the top-level (Unit-level) input and output plugs of a FireWire audio device. It achieves this by using a `DescriptorAccessor` to read and interpret "Unit Plug Information" descriptors from the device's configuration ROM.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<vector>`: For `std::vector`.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<expected>`: For `std::expected` error handling.
    -   `"DescriptorAccessor.hpp"`: For the `DescriptorAccessor` class, used to read raw descriptor data.
    -   `"AudioPlug.hpp"`: For the `AudioPlug` class, as this discoverer will create instances of it for audio plugs.
    -   `"Enums.hpp"`: For enums like `PlugDirection` and `PlugType` (which would differentiate between Audio, MIDI, etc., plugs at the unit level).
    -   `"Error.h"`: For `IOKitError`.
    -   `<spdlog/spdlog.h>`: For logging.
    -   (Potentially a simple struct like `MidiPlugInfo` if MIDI plugs are also handled here with basic info).

-   **Struct `ParsedPlugs` (Likely defined here or as a nested type):**
    -   A helper structure to return the results of plug discovery, potentially separating audio and MIDI plugs.
    -   Example:
        ```cpp
        struct ParsedPlugs {
            std::vector<AudioPlug> audioInputPlugs;
            std::vector<AudioPlug> audioOutputPlugs;
            // std::vector<MidiPlugInfo> midiInputPlugs; // If handling basic MIDI plug info
            // std::vector<MidiPlugInfo> midiOutputPlugs;
        };
        ```

-   **Class `UnitPlugDiscoverer`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `UnitPlugDiscoverer(std::shared_ptr<DescriptorAccessor> accessor, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the discoverer with a `DescriptorAccessor` (to read the raw unit plug information list descriptors) and a logger.
        -   **Main Discovery Method:**
            -   `std::expected<ParsedPlugs, IOKitError> discoverPlugs() const;`
            -   This is the primary public method to initiate the discovery of all unit-level plugs.
            -   **Operation (Conceptual - details in .cpp):**
                1.  Internally, it would likely call a helper method twice, once for `PlugDirection::Input` and once for `PlugDirection::Output`.
                2.  For each direction:
                    -   Uses `_accessor->readUnitPlugInfoList(direction)` to get a list of raw "Unit Plug Information Descriptors".
                    -   Iterates through each raw descriptor in the list.
                    -   For each raw descriptor, it parses fields like `plug_id`, `plug_type` (e.g., Audio, MIDI), and `number_of_channels_or_jacks` using `DescriptorUtils`.
                    -   If `plug_type` is Audio, it creates an `AudioPlug` object, initializing it with the ID, direction, and channel count. It then uses the `_accessor` again to read the associated INFO block for this plug ID (at the unit level) to get its name and sets it on the `AudioPlug` object.
                    -   If `plug_type` is MIDI, it might create a simpler `MidiPlugInfo` struct with ID, direction, number of jacks, and its name from an INFO block.
                    -   Adds the created `AudioPlug` or `MidiPlugInfo` to the appropriate vector in the `ParsedPlugs` result structure.
                3.  Returns the `ParsedPlugs` structure wrapped in `std::expected`, or an `IOKitError` if any critical step fails.

    -   **Private Members:**
        -   `std::shared_ptr<DescriptorAccessor> _accessor;`: The accessor used to read descriptor data.
        -   `std::shared_ptr<spdlog::logger> _logger;`: The logger instance.
        -   (Potentially private helper methods like `discoverPlugsForDirection(PlugDirection direction)`).

**Overall Role:**
The `UnitPlugDiscoverer` is responsible for the initial scan of a FireWire device's top-level (Unit-level) I/O connection points (plugs). It identifies:
-   The ID of each plug.
-   Whether each plug is an input or output.
-   The type of each plug (e.g., Audio or MIDI).
-   Basic characteristics like the number of channels (for audio) or jacks (for MIDI).
-   The human-readable name of each plug (from INFO blocks).
This information forms the initial set of plugs that are then typically passed to `DeviceInfo`. For `AudioPlug` objects discovered here, the `PlugDetailParser` would later be used to determine the specific audio stream formats they support.
