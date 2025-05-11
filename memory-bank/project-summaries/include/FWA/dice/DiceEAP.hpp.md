# Summary for include/FWA/dice/DiceEAP.hpp

This C++ header file defines the `FWA::Dice::DiceEAP` class. This class is responsible for managing communication with a TC Electronic DICE-based FireWire audio device using its proprietary Extended Audio Protocol (EAP). EAP is layered on top of standard AV/C vendor-unique commands and provides access to advanced device features not covered by generic AV/C.

**Key Declarations and Components:**

-   **Namespace `FWA::Dice`:** The class is defined within this nested namespace.

-   **Includes:**
    -   `../../CommandInterface.h`: For the `FWA::CommandInterface` class, used to send the underlying AV/C commands that carry EAP messages.
    -   `"DiceDefines.hpp"`: For DICE-specific constants, EAP command IDs, and data structures (like `ClockSelectCaps`, `RoutingEntry`, `GlobalStatus`).
    -   `../../Error.h`: For `IOKitError`.
    -   `<vector>`: For `std::vector`.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<expected>`: For `std::expected` error handling.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<cstdint>`: For fixed-width integer types.

-   **Class `DiceEAP`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `DiceEAP(std::shared_ptr<FWA::CommandInterface> avcCommandInterface, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the `DiceEAP` object with a `CommandInterface` (to send the AV/C commands that encapsulate EAP) and a logger.
        -   **High-Level EAP Command Methods:**
            -   The class declares a set of public methods, each corresponding to a specific EAP operation. These methods abstract the details of EAP packet construction and AV/C command wrapping.
            -   **Memory Access:**
                -   `std::expected<std::vector<uint32_t>, IOKitError> readMemory(uint32_t address, uint32_t sizeInDWords) const;`
                -   `std::expected<void, IOKitError> writeMemory(uint32_t address, const std::vector<uint32_t>& dataDWords) const;`
            -   **Clocking and Sample Rate:**
                -   `std::expected<ClockSelectCaps, IOKitError> getClockSelectCaps() const;` (Assuming `ClockSelectCaps` is a struct defined in `DiceDefines.hpp` to hold capabilities).
                -   `std::expected<uint8_t, IOKitError> getClockSelect() const;` (Returns current clock source ID).
                -   `std::expected<void, IOKitError> setClockSelect(uint8_t clockSourceID) const;`
                -   `std::expected<uint8_t, IOKitError> getNominalSampleRate() const;` (Returns current sample rate ID).
                -   `std::expected<void, IOKitError> setNominalSampleRate(uint8_t sampleRateID) const;`
            -   **Routing:**
                -   `std::expected<RoutingTable, IOKitError> getRoutingTable() const;` (Assuming `RoutingTable` is a struct/class for the routing matrix).
                -   `std::expected<void, IOKitError> setRoutingEntry(const RoutingEntry& entry) const;` (Assuming `RoutingEntry` struct).
            -   **Status:**
                -   `std::expected<GlobalStatus, IOKitError> getGlobalStatus() const;` (Assuming `GlobalStatus` struct).
            -   (Many other methods for specific EAP commands related to mixer parameters, I/O settings, firmware updates, etc., would be declared here based on the EAP specification for the targeted DICE chip).
        -   **Asynchronous Notification Handling (Conceptual):**
            -   If the DICE EAP supports asynchronous notifications from the device, this class might declare methods to register callbacks or listeners for such events. The actual handling would involve the `CommandInterface` being able to receive and dispatch AV/C vendor-unique notifications.

    -   **Private Members (Conceptual - implementation details in .cpp):**
        -   `std::shared_ptr<FWA::CommandInterface> _avcCommandInterface;`: The interface for sending AV/C commands.
        -   `std::shared_ptr<spdlog::logger> _logger;`: The logger instance.
        -   A private method like `sendCommandInternal(uint8_t eapCommandID, const std::vector<uint8_t>& eapPayload, size_t expectedResponsePayloadLength)` which would:
            -   Construct the full EAP packet (header + payload).
            -   Wrap this EAP packet into an AV/C VENDOR_DEPENDENT command using `DICE_VENDOR_ID` and a specific EAP opcode.
            -   Send it via `_avcCommandInterface->sendCommand()`.
            -   Receive the AV/C response, extract the EAP response packet.
            -   Parse the EAP response header for status and extract the EAP response payload.
            -   Return the EAP response payload or an error.

**Overall Role:**
The `DiceEAP` class is a crucial abstraction layer for interacting with the proprietary Extended Audio Protocol of DICE-based FireWire devices. It translates high-level requests (like "set sample rate" or "read routing table") into the specific EAP command sequences and manages the underlying AV/C vendor-unique command communication.
-   It is primarily used by the `DiceAudioDevice` class to implement its DICE-specific control methods.
-   It relies on `DiceDefines.hpp` for EAP command codes and data structure definitions.
-   It uses the generic `CommandInterface` to send the actual AV/C packets over FireWire.
This class is essential for unlocking and controlling the advanced functionalities of DICE hardware.
