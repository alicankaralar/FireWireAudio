# Summary for include/FWA/dice/DiceAbsoluteAddresses.hpp

This C++ header file defines the `FWA::Dice::AbsoluteAddresses` namespace. This namespace is dedicated to holding constant definitions for known absolute memory addresses of various registers and data structures on TC Electronic's DICE family of FireWire audio chipsets (e.g., DICE, DICE Jr., DICE Mini).

**Key Declarations and Components:**

-   **Namespace `FWA::Dice::AbsoluteAddresses`:** All constants are defined within this nested namespace.

-   **Includes:**
    -   `<cstdint>`: For `uint32_t` type, as these are typically 32-bit memory addresses.

-   **Constant Definitions (`static constexpr uint32_t`):**
    -   The file contains a large number of `static constexpr uint32_t` declarations, each representing a specific hardware register address. These addresses are specific to the DICE chipset architecture.
    -   **Organization:** The addresses are typically grouped by functional blocks of the DICE chip:
        -   **Global Registers:**
            -   `GLOBAL_REG_BASE`: Base address for the global register block.
            -   Offsets from this base for registers like:
                -   Chip ID / Revision.
                -   Firmware Version.
                -   Global Status / Control.
                -   Interrupt Control.
        -   **Clock Control Registers:**
            -   `CLOCK_REG_BASE`: Base address for clocking circuitry.
            -   Offsets for registers controlling:
                -   Clock Source Selection (Internal, Word Clock, ADAT, S/PDIF, etc.).
                -   Current Clock Status / Lock Indication.
                -   Nominal Sample Rate Setting (e.g., 44.1kHz, 48kHz, 96kHz).
                -   PLL (Phase-Locked Loop) control and status.
        -   **Routing Matrix Registers:**
            -   `ROUTER_REG_BASE`: Base address for the audio routing matrix.
            -   Addresses or patterns for registers that define connections between various input and output sources/destinations within the chip.
        -   **AVS (Audio Video Streaming) Interface Registers:**
            -   `AVS_RX_X_BASE`, `AVS_TX_X_BASE` (where X is an index for multiple AVS interfaces): Base addresses for isochronous stream receivers and transmitters.
            -   Offsets for registers controlling:
                -   Stream format.
                -   Channel allocation.
                -   Buffer pointers / DMA settings.
                -   Stream start/stop control.
        -   **Digital Audio Interface Registers (AES/EBU, ADAT, S/PDIF):**
            -   `AES_RX_REG_BASE`, `AES_TX_REG_BASE`.
            -   `ADAT_RX_REG_BASE`, `ADAT_TX_REG_BASE`.
            -   `SPDIF_RX_REG_BASE`, `SPDIF_TX_REG_BASE`.
            -   Offsets for registers controlling:
                -   Interface mode and status.
                -   Channel status data.
                -   Error flags.
        -   **Mixer Engine Registers:**
            -   `MIXER_ENG_X_BASE` (for multiple mixer engines): Base addresses.
            -   Offsets for registers controlling:
                -   Channel faders, mutes, solos, pans.
                -   Bus assignments.
                -   Meter values.
        -   **EAP (Extended Audio Protocol) Memory Regions:**
            -   Addresses for memory regions used by the EAP for command/response buffers or shared status information, if EAP operations involve direct memory access in addition to message passing.

**Overall Role:**
The `DiceAbsoluteAddresses.hpp` file is a critical piece of hardware-specific information. These predefined memory addresses are essential for low-level control and interaction with DICE-based audio devices.
-   They are primarily used by the `DiceEAP` class when it needs to perform direct memory reads or writes to DICE registers as part of implementing EAP commands (some EAP commands are essentially wrappers around register access).
-   They might also be used by other DICE-specific classes like `DiceRouter` or even `DiceAudioDevice` if they need to directly query or manipulate hardware state that isn't covered by standard AV/C commands or higher-level EAP messages.
Having these addresses centralized and named makes the low-level DICE interaction code more readable and maintainable. This file essentially acts as a hardware abstraction layer at the register address level for the DICE chipset.
