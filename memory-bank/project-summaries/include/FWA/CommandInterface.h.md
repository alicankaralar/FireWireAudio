# Summary for include/FWA/CommandInterface.h

This C++ header file defines the `FWA::CommandInterface` class. This class is responsible for abstracting the low-level details of sending AV/C (Audio Video Control) commands to a FireWire device and receiving responses. It primarily uses IOKit's FireWire user client APIs for this communication.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<IOKit/firewire/IOFireWireLib.h>`: For IOKit FireWire types like `IOFireWireLibDeviceRef` and AV/C command/notifier interface types.
    -   `<vector>`: For `std::vector<uint8_t>` to represent command and response data.
    -   `<expected>`: For `std::expected` error handling.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `"Error.h"`: For the `IOKitError` enum and error handling utilities.
    -   `"Enums.hpp"`: For `SubunitType`.

-   **Class `CommandInterface`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `CommandInterface(IOFireWireLibDeviceRef deviceRef, std::shared_ptr<spdlog::logger> logger);`
            -   Takes an `IOFireWireLibDeviceRef` (the IOKit handle to the specific FireWire device) and a logger.
            -   In its implementation (`.cpp` file), this constructor would:
                -   Open the FireWire user client for the device.
                -   Query for and obtain the `IOFireWireAVCCommandInterface**` (for sending commands) and potentially `IOFireWireAVCNotifierInterface**` (for asynchronous notifications).
        -   **Destructor:**
            -   `~CommandInterface();`
            -   Responsible for releasing the IOKit AV/C command/notifier interfaces and closing the FireWire user client.
        -   **Core Command Sending Method:**
            -   `std::expected<std::vector<uint8_t>, IOKitError> sendCommand(SubunitType subunitType, uint8_t subunitID, const std::vector<uint8_t>& commandData, size_t expectedResponseLength);`
            -   This is the primary method for sending an AV/C command.
            -   **Parameters:**
                -   `subunitType`: The type of the target subunit (e.g., Unit, Audio, Music).
                -   `subunitID`: The ID of the target subunit.
                -   `commandData`: A byte vector containing the AV/C command payload (opcode and operands).
                -   `expectedResponseLength`: The expected length of the response payload.
            -   **Implementation Details (in .cpp):**
                -   Constructs the full AV/C command frame (typically 512 bytes), including:
                    -   CTYPE (command type, e.g., CONTROL, STATUS, NOTIFY).
                    -   Subunit type and ID.
                    -   The actual command opcode and operands from `commandData`.
                -   Uses the `(*_avcCommandInterface)->AVCCommand(...)` IOKit function to send the command synchronously.
                -   Waits for and receives the AV/C response frame.
                -   Parses the response frame to check the AV/C response code (e.g., ACCEPTED, REJECTED, NOT_IMPLEMENTED).
                -   If the command was accepted, it extracts the response payload.
                -   Returns the response payload as a `std::vector<uint8_t>` wrapped in `std::expected`, or an `IOKitError` if the command failed at the IOKit level or was rejected by the device.
        -   **Asynchronous Notification Handling (Conceptual):**
            -   The header might declare methods for registering/unregistering AV/C notification handlers (e.g., `registerAVCNofiticationListener`, `unregisterAVCNofiticationListener`).
            -   The implementation would use the `IOFireWireAVCNotifierInterface**` to set up listeners for specific AV/C events from the device. Received notifications would then be dispatched to registered C++ callback objects or functions.

    -   **Private Members (Conceptual - implementation details in .cpp):**
        -   `_deviceRef`: `IOFireWireLibDeviceRef`.
        -   `_avcCommandInterface`: `IOFireWireAVCCommandInterface**` (the IOKit interface for sending AV/C commands).
        -   `_avcNotifierInterface`: `IOFireWireAVCNotifierInterface**` (the IOKit interface for AV/C notifications, if used).
        -   `_logger`: `std::shared_ptr<spdlog::logger>`.
        -   A run loop source or command gate if asynchronous operations are handled directly here.

**Overall Role:**
The `CommandInterface` class provides a crucial abstraction layer over the often complex and C-oriented IOKit FireWire APIs for sending AV/C commands. It simplifies interaction with the FireWire device at the command level for other parts of the FWA library, such as:
-   `DescriptorAccessor`: For reading descriptors.
-   `AudioDevice`: For sending control commands (e.g., set sample rate, set volume).
-   `AudioSubunit` / `MusicSubunit`: For sending subunit-specific commands.
By encapsulating the IOKit calls, command frame construction, and basic response parsing, it promotes cleaner and more maintainable code in the higher-level components.
