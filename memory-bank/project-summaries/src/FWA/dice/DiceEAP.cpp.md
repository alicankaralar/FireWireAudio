# Summary for src/FWA/dice/DiceEAP.cpp

This C++ file implements the `FWA::DiceEAP` class. This class is responsible for handling the DICE (Digital Interface Communication Engine) Extended Audio Protocol (EAP). EAP is a proprietary protocol developed by TC Electronic, layered on top of AV/C vendor-unique commands, to provide advanced control and status querying for DICE-based FireWire audio devices.

**Key Functionalities:**

-   **Constructor (`DiceEAP::DiceEAP`):**
    -   Takes a `std::shared_ptr<CommandInterface>` (for sending the underlying AV/C vendor-unique commands) and an `std::shared_ptr<spdlog::logger>`.
    -   Stores these dependencies.

-   **Core EAP Command Execution (`sendCommandInternal` - likely private):**
    -   This private method would be the core for sending any EAP command.
    -   **EAP Packet Construction:** It takes an EAP command ID and an EAP payload (as `std::vector<uint8_t>`). It then constructs the full EAP packet, which typically includes:
        -   A header (e.g., sequence number, length).
        -   The EAP command ID.
        -   The EAP payload.
    -   **AV/C Vendor-Unique Command Wrapping:** The constructed EAP packet is then wrapped within an AV/C vendor-unique command frame. This involves:
        -   Setting the AV/C command type to VENDOR_DEPENDENT (0x00).
        -   Using the TC Electronic/DICE vendor ID (typically `0x000139`).
        -   Using a specific opcode for EAP communication (e.g., `0xEC` as seen in some DICE implementations).
        -   The EAP packet becomes the payload of this AV/C command.
    -   **Sending via `CommandInterface`:** Uses `_commandInterface->sendCommand()` to send the AV/C vendor-unique command to the device (usually to the Unit subunit, ID 0x1F).
    -   **Response Parsing:**
        -   Receives the AV/C response.
        -   Extracts the EAP response packet from the AV/C response payload.
        -   Parses the EAP response header to check for EAP-level success or error codes.
        -   Extracts the EAP response payload if the command was successful.
    -   Returns a `std::expected` containing the EAP response payload or an error code.

-   **Public EAP Command Methods:**
    -   The class exposes public methods that wrap common EAP operations. These methods internally call `sendCommandInternal` with the appropriate EAP command ID and payload. Examples:
        -   `std::expected<std::vector<uint32_t>, IOKitError> readMemory(uint32_t address, uint32_t sizeInDWords)`: Sends an EAP command to read a block of memory from the DICE chip.
        -   `std::expected<void, IOKitError> writeMemory(uint32_t address, const std::vector<uint32_t>& dataDWords)`: Sends an EAP command to write a block of memory.
        -   `std::expected<ClockSelectCaps, IOKitError> getClockSelectCaps()`: Gets the available clock sources and their names.
        -   `std::expected<uint8_t, IOKitError> getClockSelect()`: Gets the currently selected clock source ID.
        -   `std::expected<void, IOKitError> setClockSelect(uint8_t clockSourceID)`: Sets the clock source.
        -   `std::expected<uint8_t, IOKitError> getNominalSampleRate()`: Gets the current sample rate ID (SFC).
        -   `std::expected<void, IOKitError> setNominalSampleRate(uint8_t sampleRateID)`: Sets the sample rate.
        -   `std::expected<GlobalStatus, IOKitError> getGlobalStatus()`: Retrieves various global status flags from the device.
        -   `std::expected<RoutingTable, IOKitError> getRoutingTable()`: Reads the device's internal routing matrix.
        -   `std::expected<void, IOKitError> setRoutingEntry(uint16_t outputChannel, uint16_t inputChannel, bool connect)`: Modifies a connection in the routing matrix.
        -   Other methods for controlling mixer parameters, phantom power, input sensitivity, etc., if exposed by the device's EAP implementation.

-   **Asynchronous EAP Notification Handling (Conceptual):**
    -   A complete EAP implementation would also need to handle asynchronous EAP notifications sent by the device (e.g., when a parameter is changed locally on the hardware or by another controller).
    -   This would involve the `CommandInterface` being able to register for specific AV/C vendor-unique notifications and dispatching the EAP notification payloads to the `DiceEAP` class for parsing and further propagation (e.g., via callbacks or a listener pattern).

**Overall Role:**
The `DiceEAP` class is a critical component for interacting with DICE-based FireWire audio devices. It encapsulates the proprietary EAP protocol, allowing the `DiceAudioDevice` class (and potentially other high-level logic) to control advanced features like clocking, sample rates, routing, and mixer settings that are not typically accessible through standard AV/C commands. It acts as a translator between high-level control requests and the specific EAP messages embedded within AV/C vendor-unique commands.
