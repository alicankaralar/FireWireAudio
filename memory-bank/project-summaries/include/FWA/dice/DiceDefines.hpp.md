# Summary for include/FWA/dice/DiceDefines.hpp

This C++ header file defines various constants, enumerations, and simple data structures that are specific to TC Electronic's DICE (Digital Interface Communication Engine) chipset and its proprietary Extended Audio Protocol (EAP). These definitions are used by other DICE-specific classes within the FWA library (like `DiceEAP`, `DiceRouter`, and `DiceAudioDevice`) to interact with DICE hardware.

**Key Declarations and Components:**

-   **Namespace `FWA::Dice`:** All definitions are encapsulated within this nested namespace to avoid conflicts and clearly indicate their DICE-specific nature.

-   **Includes:**
    -   `<cstdint>`: For fixed-width integer types like `uint8_t`, `uint16_t`, `uint32_t`.
    -   (Potentially other standard headers if needed by struct definitions, e.g., `<string>`, `<vector>`, `<array>`).

-   **Constants (`static constexpr` or `#define`):**
    -   **Vendor ID:**
        -   `DICE_VENDOR_ID = 0x000139;` (The IEEE 1394 registered Vendor ID for TC Electronic).
    -   **EAP Command Opcodes/IDs:**
        -   Constants representing the command codes for various EAP operations, e.g.:
            -   `EAP_CMD_READ_MEMORY`
            -   `EAP_CMD_WRITE_MEMORY`
            -   `EAP_CMD_GET_CLOCK_SELECT_CAPS`
            -   `EAP_CMD_GET_CLOCK_SELECT`
            -   `EAP_CMD_SET_CLOCK_SELECT`
            -   `EAP_CMD_GET_NOMINAL_SAMPLE_RATE`
            -   `EAP_CMD_SET_NOMINAL_SAMPLE_RATE`
            -   `EAP_CMD_GET_ROUTING_TABLE`
            -   `EAP_CMD_SET_ROUTING_ENTRY`
            -   `EAP_CMD_GET_GLOBAL_STATUS`
            -   (And many others for mixer controls, specific parameter access, etc.)
    -   **EAP Packet Structure Constants:**
        -   Offsets or sizes for fields within EAP command/response packets.
        -   Magic numbers or specific values used in EAP communication.
    -   **Register Value Constants:**
        -   Specific bitmasks or values used when reading from or writing to DICE hardware registers via EAP memory access commands.

-   **Enumerations (`enum class`):**
    -   **`ClockSourceID : uint8_t` (or similar):**
        -   Enumerates known clock sources available on DICE devices (e.g., `INTERNAL_QUARTZ`, `WORD_CLOCK_IN`, `ADAT_IN_1`, `SPDIF_IN`, `AVS_RX_1_SYNC`, etc.). The actual integer values would correspond to those expected by the DICE firmware.
    -   **`SampleRateID : uint8_t` (or similar):**
        -   Enumerates DICE-specific identifiers for different sample rates (e.g., `DICE_SR_44100`, `DICE_SR_48000`, `DICE_SR_96000`). These might map to the SFC codes from `FWA::Enums` or be distinct.
    -   **`EAPStatus : uint8_t` (or similar):**
        -   Defines status codes returned in EAP responses (e.g., `EAP_SUCCESS`, `EAP_ERROR_INVALID_PARAM`, `EAP_ERROR_BUSY`).
    -   Other enums for specific DICE parameters or states.

-   **Simple Data Structures (`struct`):**
    -   These structs are often used to represent the payload of EAP commands or responses, or to group related DICE parameters.
    -   **`EAPHeader` (Conceptual):** A struct to represent the common header of an EAP packet (e.g., sequence number, command ID, payload length).
    -   **`ClockSelectCapability`:** A struct to hold information about a single available clock source (e.g., its ID, a string name).
    -   **`RoutingEntry`:** A struct to represent a single connection in the routing matrix (e.g., `uint16_t sourceID; uint16_t destinationID; bool isConnected;`).
    -   **`GlobalStatusFlags`:** A struct with boolean members representing various global status indicators read from the device.
    -   Other small POD (Plain Old Data) structs tailored to specific EAP command/response data structures.

**Overall Role:**
The `DiceDefines.hpp` file serves as a central repository for all the hardware-specific constants, enumerations, and simple data structures related to the TC Electronic DICE chipset and its EAP.
-   **Abstraction:** It abstracts away raw magic numbers and bit-level details, providing named constants and typed structures.
-   **Readability and Maintainability:** Makes the code in `DiceEAP.cpp`, `DiceRouter.cpp`, and `DiceAudioDevice.cpp` (which interact with DICE hardware) much easier to read, understand, and maintain.
-   **Type Safety:** Using enums and structs improves type safety compared to using raw integers or byte arrays directly.
-   **Centralization:** Consolidates DICE-specific knowledge in one place, making it easier to update if new DICE features or revisions are supported.
This file is essential for any part of the FWA library that needs to perform low-level communication or control of DICE-based audio interfaces.
