# Summary for src/FWA/AudioDevice.cpp

This C++ file implements the `FWA::AudioDevice` class, which serves as a base representation for a FireWire audio device within the user-space FWA library. It encapsulates common functionalities for interacting with and querying information from a FireWire device.

**Key Functionalities:**

-   **Constructor and Initialization:**
    -   The constructor takes an `IOFireWireLibDeviceRef` (the IOKit interface to the physical device), the device's GUID, name, vendor name, and an `spdlog::logger`.
    -   It initializes member variables like `_guid`, `_name`, `_vendorName`, `_ioDeviceRef`, and the logger.
    -   `init(bool discoverSubunitsAndParse)`:
        -   Performs further initialization.
        -   Creates a `CommandInterface` instance, which is used for sending AV/C commands to the device.
        -   If `discoverSubunitsAndParse` is true (default):
            -   Calls `discoverSubunits()` to find audio and music subunits on the device.
            -   Calls `parseDescriptors()` to read and interpret various descriptors from the device's configuration ROM (e.g., unit info, subunit info, plug info, stream formats).
        -   Sets an initialization flag `_isInitialized`.

-   **Device Information Accessors:**
    -   Provides getter methods for various device properties:
        -   `getGUID()`, `getName()`, `getVendorName()`, `getModelName()`, `getFirmwareVersion()`, `getSerialNumber()`.
        -   Some of these (like model, firmware, serial) are populated by `parseDescriptors()`.
    -   `getDeviceInfoAsJson()`: Serializes key device information into a JSON string using `nlohmann::json`. This includes basic info, plug details, and subunit capabilities.

-   **Subunit Discovery and Management:**
    -   `discoverSubunits()`:
        -   Uses a `SubunitDiscoverer` instance to find AV/C subunits.
        -   Specifically looks for an audio subunit (`kAudioSubunitType`) and a music subunit (`kMusicSubunitType`).
        -   If found, it creates `AudioSubunit` and `MusicSubunit` objects respectively and stores them in `_audioSubunit` and `_musicSubunit`.
    -   `getAudioSubunit()` and `getMusicSubunit()`: Return shared pointers to the discovered subunits.

-   **Descriptor Parsing:**
    -   `parseDescriptors()`:
        -   Uses a `DescriptorReader` (initialized with the `CommandInterface`) to read raw descriptor data from the device.
        -   Uses a `DeviceParser` to interpret these raw descriptors and populate internal structures like:
            -   `_unitInfo`: General unit information.
            -   `_inputPlugs`, `_outputPlugs`: Vectors of `AudioPlug` objects, storing details about each physical input/output plug on the device (e.g., type, channels, supported formats).
            -   `_audioSubunitInfo`, `_musicSubunitInfo`: Information specific to the audio and music subunits.
        -   It also attempts to parse more detailed plug information using `PlugDetailParser` if available.

-   **Stream and Plug Information:**
    -   `getInputPlugs()`, `getOutputPlugs()`: Return vectors of discovered audio plugs.
    -   `getPlugByID(uint8_t id, bool isInput)`: Finds a specific plug by its ID.
    -   `getAvailableInputFormats()`, `getAvailableOutputFormats()`: (Conceptual, based on parsed plug info) Would provide information about supported audio formats.

-   **Command Interface:**
    -   `getCommandInterface()`: Returns a shared pointer to the `CommandInterface` object, allowing other components to send AV/C commands to this device.

-   **State Management:**
    -   `start()` and `stop()`: Placeholder methods for managing the operational state of the device (e.g., starting/stopping audio streams), though the detailed implementation might be in derived classes or stream handlers.
    -   `isInitialized()`: Returns the initialization status.

**Overall Role:**
The `FWA::AudioDevice` class is a central abstraction in the user-space library. It represents a discovered FireWire audio device and provides a unified way to:
1.  Access its basic identification (GUID, name, vendor).
2.  Discover its capabilities by parsing its configuration ROM descriptors (subunits, plugs, formats).
3.  Obtain interfaces to its subunits (audio, music).
4.  Provide a command interface for sending AV/C commands.
It serves as a base for more specialized device classes (like `DiceAudioDevice`) and provides the necessary information for higher-level components (like stream managers or GUI controllers) to interact with the hardware.
