# Summary for include/FWA/AudioDevice.h

This C++ header file defines the `FWA::AudioDevice` class, which is a central and crucial abstraction in the FWA library. It represents a single FireWire audio device and encapsulates its properties, capabilities, and control interfaces.

**Key Components and Declared Functionalities:**

-   **Includes:**
    -   Standard library headers (`string`, `vector`, `memory`, `optional`, `expected`).
    -   Project-specific headers for dependencies like `DeviceInfo.hpp`, `AudioSubunit.hpp`, `MusicSubunit.hpp`, `CommandInterface.h`, `Enums.hpp`, `Error.h`, `spdlog`, `nlohmann/json`.
    -   IOKit headers (`IOKit/firewire/IOFireWireLib.h`).

-   **Class `AudioDevice`:**
    -   **Member Variables (Protected or Private):**
        -   `_ioDevice`: `IOFireWireLibDeviceRef` (the IOKit handle to the physical device).
        -   `_guid`: `uint64_t` (Global Unique Identifier).
        -   `_name`, `_vendorName`, `_modelName`: `std::string`.
        -   `_deviceInfo`: `DeviceInfo` object (stores parsed device details).
        -   `_commandInterface`: `std::shared_ptr<CommandInterface>` (for sending AV/C commands).
        -   `_subunitDiscoverer`: `std::shared_ptr<SubunitDiscoverer>`.
        -   `_deviceParser`: `std::shared_ptr<DeviceParser>`.
        -   `_audioSubunit`: `std::shared_ptr<AudioSubunit>`.
        -   `_musicSubunit`: `std::shared_ptr<MusicSubunit>`.
        -   `_isoStreamHandler`: `std::shared_ptr<Isoch::IsoStreamHandler>` (manages audio streams).
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.
        -   `_isValid`: `bool` (indicates if the device object is properly initialized).
        -   (Potentially members for current sample rate, clock source, etc.)

    -   **Constructor:**
        -   `AudioDevice(IOFireWireLibDeviceRef device, uint64_t guid, std::string name, std::string vendor, std::shared_ptr<spdlog::logger> logger);`
        -   Initializes basic properties and creates core components like `CommandInterface`.

    -   **Destructor:**
        -   `virtual ~AudioDevice();` (virtual for potential inheritance, e.g., `DiceAudioDevice`).
        -   Handles cleanup, like stopping I/O and releasing resources.

    -   **Initialization (`init`):**
        -   `virtual std::expected<void, IOKitError> init();`
        -   This is a critical method called after construction. It performs:
            -   Subunit discovery using `_subunitDiscoverer` to find Audio and Music subunits.
            -   Creation of `AudioSubunit` and `MusicSubunit` objects if found.
            -   Comprehensive descriptor parsing using `_deviceParser` to populate the `_deviceInfo` object (including plug details, stream formats, subunit capabilities).
            -   Initialization of the `_isoStreamHandler`.
            -   Sets `_isValid` to true on success.

    -   **Device Information Accessors:**
        -   `uint64_t getGUID() const;`
        -   `const std::string& getName() const;`
        -   `const std::string& getVendorName() const;`
        -   `const std::string& getModelName() const;`
        -   `const std::string& getSerialNumber() const;`
        -   `const std::string& getFirmwareVersion() const;`
        -   `const DeviceInfo& getDeviceInfo() const;`
        -   `virtual nlohmann::json getDeviceInfoAsJson() const;` (virtual to allow derived classes like `DiceAudioDevice` to add specific info).

    -   **Subunit Accessors:**
        -   `std::shared_ptr<AudioSubunit> getAudioSubunit() const;`
        -   `std::shared_ptr<MusicSubunit> getMusicSubunit() const;`

    -   **Control Interface:**
        -   `std::shared_ptr<CommandInterface> getCommandInterface() const;`

    -   **State and Validity:**
        -   `bool isValid() const;`

    -   **Audio I/O Control (Likely delegating to `_isoStreamHandler`):**
        -   `virtual std::expected<void, IOKitError> startIO();`
        -   `virtual std::expected<void, IOKitError> stopIO();`
        -   `virtual bool isIOStarted() const;`
        -   (Methods to create/destroy input/output streams, e.g., `createOutputStream`, `createInputStream`, would also likely delegate to `_isoStreamHandler`).

    -   **Device Property Control (Virtual for device-specific implementations):**
        -   `virtual std::expected<double, IOKitError> getSampleRate() const;`
        -   `virtual std::expected<void, IOKitError> setSampleRate(double rate);`
        -   `virtual std::expected<std::vector<double>, IOKitError> getAvailableSampleRates() const;`
        -   `virtual std::expected<uint32_t, IOKitError> getClockSource() const;`
        -   `virtual std::expected<void, IOKitError> setClockSource(uint32_t sourceID);`
        -   `virtual std::expected<std::map<uint32_t, std::string>, IOKitError> getAvailableClockSources() const;`
        -   (Volume, mute, and other controls would follow a similar pattern).

    -   **XPC Interaction (Conceptual):**
        -   While not directly exposed in the public API here, internal methods might interact with `XPCBridge` to:
            -   Notify the `FWADaemon` about stream start/stop requests that need shared memory coordination.
            -   Respond to control requests forwarded from the daemon (originating from the driver).

**Overall Role:**
The `AudioDevice` class is the primary C++ representation of a connected FireWire audio device within the FWA library. It encapsulates:
-   Discovery and parsing of device capabilities (descriptors, subunits, plugs, formats).
-   Access to device information.
-   A command interface for sending AV/C commands.
-   Management of audio streams (via `IsoStreamHandler`).
-   Control over device parameters like sample rate and clock source.
It serves as the main object through which client code (like `DeviceController` or a C-API wrapper) interacts with a specific hardware device. The use of `virtual` methods for controls allows for specialization by derived classes like `DiceAudioDevice` to handle vendor-specific implementations.
