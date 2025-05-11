# Summary for include/FWA/dice/DiceAudioDevice.h

This C++ header file defines the `FWA::DiceAudioDevice` class. This class is a specialized version of the base `FWA::AudioDevice` class, tailored specifically for interacting with FireWire audio devices that are built upon TC Electronic's DICE (Digital Interface Communication Engine) chipset family (e.g., DICE, DICE Jr., DICE Mini). It inherits from `AudioDevice` and overrides or extends its functionality to support DICE-specific features and control mechanisms, primarily through the Extended Audio Protocol (EAP).

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.
    -   **Nested Namespace `Dice`:** DICE-specific components like `DiceEAP` and `DiceRouter` are often in a nested `Dice` namespace.

-   **Includes:**
    -   `../AudioDevice.h`: For the base `AudioDevice` class.
    -   `"DiceEAP.hpp"`: For the `DiceEAP` class, which handles EAP communication.
    -   `"DiceRouter.hpp"`: For the `DiceRouter` class, which manages the device's routing matrix.
    -   `"DiceDefines.hpp"`: For DICE-specific constants and enums (e.g., EAP command IDs, clock source IDs).
    -   (Other standard includes like `<memory>`, `<vector>`, `<string>`, `<expected>`, `<spdlog/spdlog.h>`, `<nlohmann/json.hpp>` are inherited or included via `AudioDevice.h`).

-   **Class `DiceAudioDevice` (publicly inherits from `AudioDevice`):**
    -   **Public Interface:**
        -   **Constructor:**
            -   `DiceAudioDevice(IOFireWireLibDeviceRef deviceRef, uint64_t guid, std::string name, std::string vendor, std::shared_ptr<spdlog::logger> logger);`
            -   Calls the base `AudioDevice` constructor with the provided parameters.
        -   **Initialization (Override):**
            -   `std::expected<void, IOKitError> init() override;`
            -   This method first calls `AudioDevice::init()` to perform common device initialization (subunit discovery, basic descriptor parsing).
            -   Then, it performs DICE-specific initialization:
                -   Creates and initializes `_diceEAP = std::make_unique<DiceEAP>(_commandInterface, _logger);`.
                -   Creates and initializes `_diceRouter = std::make_unique<DiceRouter>(_diceEAP, _logger);`.
                -   May use `_diceEAP` to query initial DICE-specific status (e.g., EAP version, current clock, sample rate, routing table).
        -   **Overridden Control Methods:**
            -   It overrides virtual methods from `AudioDevice` to provide DICE-specific implementations using EAP commands via `_diceEAP` or `_diceRouter`.
            -   `std::expected<double, IOKitError> getSampleRate() const override;`
            -   `std::expected<void, IOKitError> setSampleRate(double rate) override;`
            -   `std::expected<std::vector<double>, IOKitError> getAvailableSampleRates() const override;`
            -   `std::expected<uint32_t, IOKitError> getClockSource() const override;`
            -   `std::expected<void, IOKitError> setClockSource(uint32_t sourceID) override;`
            -   `std::expected<std::map<uint32_t, std::string>, IOKitError> getAvailableClockSources() const override;`
        -   **New DICE-Specific Methods:**
            -   Methods for interacting with the routing matrix (delegating to `_diceRouter`):
                -   `std::expected<DiceRouter::RoutingTable, IOKitError> getRouterMatrix() const;`
                -   `std::expected<void, IOKitError> setRoutingConnection(uint16_t outputChannelID, uint16_t inputChannelID, bool connect);`
            -   Potentially other methods for controlling DICE-specific parameters exposed via EAP (e.g., mixer levels, phantom power, input sensitivity, if the EAP implementation supports them).
        -   **JSON Serialization (Override):**
            -   `nlohmann::json getDeviceInfoAsJson() const override;`
            -   Calls `AudioDevice::getDeviceInfoAsJson()` to get the base device information.
            -   Appends DICE-specific information to the JSON object, such as the routing table, detailed clock source information obtained via EAP, EAP version, etc.

    -   **Private Members:**
        -   `std::unique_ptr<DiceEAP> _diceEAP;`: Manages EAP communication.
        -   `std::unique_ptr<DiceRouter> _diceRouter;`: Manages the routing matrix.

**Overall Role:**
The `DiceAudioDevice` class extends the generic `AudioDevice` functionality to provide specialized support for TC Electronic DICE-based hardware. It leverages the `DiceEAP` and `DiceRouter` classes to:
1.  Communicate with the device using the proprietary Extended Audio Protocol (EAP) for advanced control and status.
2.  Manage and control the device's internal signal routing matrix.
3.  Provide access to DICE-specific features like detailed clock source selection and sample rate control that go beyond standard AV/C mechanisms.
This class allows the FWA library to offer a richer and more complete control experience for this popular family of FireWire audio chipsets.
