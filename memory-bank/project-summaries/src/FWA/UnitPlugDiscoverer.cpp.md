# Summary for src/FWA/UnitPlugDiscoverer.cpp

This C++ file implements the `FWA::UnitPlugDiscoverer` class. The purpose of this class is to discover the top-level (Unit-level) input and output plugs of a FireWire device. It does this by reading and parsing "Unit Plug Information" descriptors from the device's configuration ROM.

**Key Functionalities:**

-   **Constructor (`UnitPlugDiscoverer::UnitPlugDiscoverer`):**
    -   Takes a `std::shared_ptr<DescriptorAccessor>` (used for reading raw descriptor data) and an `std::shared_ptr<spdlog::logger>`.
    -   Stores these dependencies.

-   **Primary Discovery Method (`discoverPlugs`):**
    -   `std::expected<ParsedPlugs, IOKitError> discoverPlugs() const`:
        -   (The return type `ParsedPlugs` is likely a struct or pair containing two vectors: one for audio plugs and one for MIDI plugs/info).
        -   This method orchestrates the discovery of both input and output plugs at the unit level.
        -   It calls internal helper methods like `discoverPlugsForDirection(PlugDirection::Input)` and `discoverPlugsForDirection(PlugDirection::Output)`.

-   **Direction-Specific Discovery (`discoverPlugsForDirection` - private helper):**
    -   `std::expected<std::vector<std::variant<AudioPlug, MidiPlugInfo>>, IOKitError> discoverPlugsForDirection(PlugDirection direction) const`:
        -   **Parameter:** `direction` (`PlugDirection::Input` or `PlugDirection::Output`).
        -   **Read Unit Plug Information Descriptor List:**
            -   It calls `_descriptorAccessor->readUnitPlugInfoList(direction)`. This method in `DescriptorAccessor` is responsible for reading the list of "Unit Plug Information Descriptors" for the specified `direction` from the device's unit level. Each entry in this list describes a single unit-level plug.
        -   **Iterate and Parse:**
            -   If the list is read successfully, it iterates through each raw unit plug information descriptor.
            -   For each descriptor, it parses key fields:
                -   `plug_id`: The ID of the plug.
                -   `plug_type`: Indicates if it's an Audio plug (0x01) or a MIDI plug (0x02), or other types.
                -   `number_of_channels_or_jacks`: For audio plugs, this is the number of channels. For MIDI plugs, it's often the number of MIDI jacks associated with this plug ID.
            -   **Object Creation:**
                -   If `plug_type` is Audio, it creates an `AudioPlug` object, initializing it with the ID, direction, and channel count.
                -   If `plug_type` is MIDI, it creates a `MidiPlugInfo` struct (or similar), storing the ID, direction, and number of jacks.
            -   **Read INFO Block for Plug Name:**
                -   It then attempts to read an INFO block descriptor associated with this `plug_id` (at the unit level) to get a textual name for the plug.
                -   If successful, the name is set on the `AudioPlug` object or stored with the `MidiPlugInfo`.
            -   The created `AudioPlug` or `MidiPlugInfo` is added to a result vector (likely a `std::vector<std::variant<AudioPlug, MidiPlugInfo>>` or separate vectors).
        -   **Return Value:** Returns an `std::expected` containing the vector of discovered plug information for that direction, or an `IOKitError`.

**Overall Role:**
The `UnitPlugDiscoverer` class is responsible for the initial discovery of all top-level physical input and output connection points (plugs) on the FireWire device. It determines whether each plug is for audio or MIDI and extracts basic information like ID, channel/jack count, and name.
This information is crucial for:
1.  Populating the `DeviceInfo` object with the list of available plugs.
2.  Providing the necessary plug IDs to `PlugDetailParser` so it can then read more specific descriptors (like stream formats for audio plugs) associated with each discovered unit-level plug.
It works in concert with `DescriptorAccessor` to fetch the data and `DescriptorUtils` to parse common fields.
