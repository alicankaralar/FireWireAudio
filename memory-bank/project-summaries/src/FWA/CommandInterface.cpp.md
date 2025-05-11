# Summary for src/FWA/CommandInterface.cpp

This C++ file implements the `FWA::CommandInterface` class. This class is responsible for managing the AV/C (Audio Video Control) command interface to a specific FireWire device. It allows sending AV/C commands to the device and receiving responses.

**Key Functionalities:**

-   **Constructor (`CommandInterface::CommandInterface`):**
    -   Takes an `IOFireWireLibDeviceRef` (the IOKit interface to the physical FireWire device) and an `spdlog::logger`.
    -   Stores the `_ioDeviceRef` and logger.
    -   **Opens AV/C Command Interface:** It attempts to open an AV/C command interface to the device using `(*_ioDeviceRef)->CreateAVCCommandInterface(_ioDeviceRef, &_avcInterface)`. The `_avcInterface` (an `IOFireWireAVCCommandInterface**`) is the IOKit object used to send and receive AV/C commands.
    -   If opening the interface fails, it logs an error.

-   **Destructor (`CommandInterface::~CommandInterface`):**
    -   If `_avcInterface` is valid (was successfully opened):
        -   Calls `(*_avcInterface)->Close(_avcInterface)` to close the command interface.
        -   Calls `(*_avcInterface)->Release(_avcInterface)` to release the IOKit object.

-   **Command Sending (`sendCommand`):**
    -   `std::expected<std::vector<uint8_t>, IOKitError> sendCommand(const std::vector<uint8_t>& command, SubunitType subunitType, uint8_t subunitID, uint32_t timeoutMs = kDefaultTimeoutMs)`:
        -   This is the primary method for sending an AV/C command and receiving its response synchronously.
        -   **Parameters:**
            -   `command`: A `std::vector<uint8_t>` containing the raw bytes of the AV/C command frame to be sent.
            -   `subunitType`: An enum `FWA::SubunitType` (e.g., `Audio`, `Music`, `Unit`) specifying the target subunit type.
            -   `subunitID`: The ID of the target subunit (0-7, or `kUnitSubunitID` for the unit itself).
            -   `timeoutMs`: Optional timeout for the command in milliseconds.
        -   **Command Preparation:**
            -   Creates an `IOFWAVCCommand` object using `(*_avcInterface)->CreateCommand(_avcInterface)`.
            -   Sets the command data using `(*commandRef)->SetCommand(commandRef, const_cast<UInt8*>(command.data()), command.size())`.
            -   Sets the target subunit type and ID using `(*commandRef)->SetSubunit(commandRef, static_cast<UInt32>(subunitType), subunitID)`.
        -   **Command Submission:**
            -   Submits the command synchronously using `(*_avcInterface)->SubmitCommand(_avcInterface, commandRef)`. This call blocks until a response is received or a timeout occurs.
        -   **Response Handling:**
            -   Gets the response status using `(*commandRef)->GetRespStatus(commandRef)`.
            -   If the status indicates success (`kIOReturnSuccess`):
                -   Gets the response data length using `(*commandRef)->GetRespLen(commandRef)`.
                -   Gets the response data pointer using `(*commandRef)->GetResp(commandRef)`.
                -   Copies the response data into a `std::vector<uint8_t>` and returns it.
            -   If there's an error or timeout, it logs the error and returns an `IOKitError`.
        -   Releases the `IOFWAVCCommand` object using `(*commandRef)->Release(commandRef)`.

-   **Asynchronous Command Sending (`sendCommandAsync` - Placeholder):**
    -   `std::expected<void, IOKitError> sendCommandAsync(...)`:
        -   This method is declared but its implementation is currently a placeholder, logging "Async command sending not yet implemented" and returning `IOKitError::Unsupported`. This indicates that fully asynchronous AV/C command handling (which would involve callbacks) is not yet completed in this class.

-   **Interface Validity Check (`isValid() const`):**
    -   Returns `true` if `_avcInterface` is not null (i.e., if it was successfully opened), `false` otherwise.

**Overall Role:**
The `CommandInterface` class provides a crucial abstraction layer for sending low-level AV/C commands to a FireWire device. It handles the IOKit specifics of creating, configuring, submitting, and receiving responses for AV/C commands. This allows other parts of the FWA library (like `DescriptorReader`, `AudioSubunit`, `DiceEAP`) to interact with the device at a higher level without needing to manage the `IOFireWireAVCCommandInterface` directly. The synchronous `sendCommand` is the primary mode of operation shown.
