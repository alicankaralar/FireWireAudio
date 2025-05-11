# Summary for src/FWA/AudioSubunit.cpp

This C++ file implements the `FWA::AudioSubunit` class. This class represents an AV/C (Audio Video Control) Audio Subunit discovered on a FireWire device. It inherits from a more generic `FWA::Subunit` base class and specializes in handling audio-specific functionalities and descriptors.

**Key Functionalities:**

-   **Constructor:**
    -   `AudioSubunit(uint8_t id, std::shared_ptr<CommandInterface> cmdInterface, std::shared_ptr<spdlog::logger> logger)`:
        -   Initializes the subunit with its `id` (obtained during subunit discovery), a shared pointer to the `CommandInterface` (for sending AV/C commands to this subunit), and a logger.
        -   Calls the base class (`Subunit`) constructor, passing `SubunitType::Audio` and the ID.

-   **Descriptor Parsing (`parseDescriptors(DescriptorReader& reader)`):**
    -   This is a crucial method overridden from the `Subunit` base class.
    -   It uses the provided `DescriptorReader` (which is capable of fetching descriptor data from the device via the `CommandInterface`) to read and interpret descriptors specifically relevant to an Audio Subunit. This typically includes:
        -   **Audio Plug Descriptors:** To identify and characterize the audio input and output plugs associated with this subunit. For each plug, it would determine its ID, direction, number of channels, and potentially a name if provided by the descriptor. This information is likely stored in a collection of `AudioPlug` objects.
        -   **Stream Format Descriptors:** To determine the audio formats (sample rates, bit depths, channel configurations) supported by the plugs connected to this subunit. This populates lists of `AudioStreamFormat` objects.
        -   **Cluster EID (Extended ID) Information:** If the subunit uses clusters for grouping related plugs or controls.
        -   **Jack Description Descriptors:** To get information about the physical connectors (e.g., XLR, RCA, TRS).
        -   **Sampling Rate Converter (SRC) Information Descriptors:** If the subunit has built-in SRC capabilities.
        -   **Effect Control Information Descriptors:** If the subunit supports audio effects that can be controlled.
    -   The parsed information is stored in member variables of the `AudioSubunit` class (e.g., `_audioPlugs`, `_supportedSampleRates`, `_supportedBitDepths`).

-   **Capability Reporting:**
    -   `getAudioPlugs() const`: Returns a list/vector of `AudioPlug` objects associated with this subunit.
    -   `getSupportedSampleRates() const`: Returns a list of supported sample rates.
    -   `isSampleRateSupported(double rate) const`: Checks if a specific sample rate is supported.
    -   Similar methods for bit depths and other audio characteristics.

-   **Control Functions (Conceptual):**
    -   While not explicitly detailed in every `AudioSubunit.cpp` (as some control might be device-specific or handled at a higher level), an Audio Subunit class is the logical place to implement methods for controlling audio-specific parameters. This could include:
        -   Setting volume or mute for specific plugs.
        -   Selecting input sources.
        -   Configuring SRCs or effects.
    -   These control functions would be implemented by constructing and sending appropriate AV/C commands to this subunit via the `_commandInterface`.

-   **JSON Serialization (`toJson() const`):**
    -   Overrides the base `Subunit::toJson()` method to add audio-specific information to the JSON representation.
    -   This would include serializing its list of `AudioPlug` objects and other relevant audio capabilities.

**Overall Role:**
The `FWA::AudioSubunit` class provides an object-oriented representation of an AV/C Audio Subunit. It's responsible for:
1.  Understanding its own capabilities by parsing its specific descriptors.
2.  Exposing these capabilities (plugs, supported formats, etc.) to the rest of the FWA library.
3.  Providing an interface (potentially) for controlling audio-related functions of that subunit.
It works in conjunction with the `AudioDevice` class, which discovers and instantiates `AudioSubunit` objects.
