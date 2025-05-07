---
**STATUS UPDATE (2025-05-06):** Significant portions of this plan, particularly concerning the `firewire_scanner` tool, have been implemented.
Key completed aspects for the scanner include:
*   Improved read reliability due to dynamic base address discovery.
*   Validated data interpretation for stream counts and sizes (dynamic counts are now used).
*   Comprehensive content validation for read register values.
*   Removal of artificial limits in stream count determination for the scanner.
The `firewire_scanner` now demonstrates more robust RX/TX stream register reading. Further work on block reads and core `DiceAudioDevice` improvements may still be pending.
---
# Plan: Improve DICE Register Reading

**Goal:** Improve the `firewire_scanner` and the underlying `DiceAudioDevice` class to read the device's capabilities (like TX/RX stream counts) accurately and efficiently, removing artificial limits and addressing unreliable reads and potentially incorrect data interpretation.

**Phase 1: Information Gathering & Analysis**

1.  **Review Current Code:**
    *   Examine `firewire_scanner.cpp`: Understand interaction with `DiceAudioDevice` and display logic.
    *   Examine `DiceAudioDevice.cpp`: Focus on `initIoFunctions`, `readDiceRegisters`, `readReg`, `writeReg`, and FireWire address mapping logic.
    *   Examine `DiceDefines.hpp`: Identify current register definitions.
2.  **Consult Documentation:**
    *   Finish reading `docs/dice-datasheet.pdf` (Chapters 5 & 6) for register details.
    *   **Crucially:** Search datasheet, `libffado` code, or other project docs for **FireWire address mapping** of DICE registers (logical base `0xCE00 0000`).
3.  **Targeted Reads:**
    *   Re-investigate `CSR STATE_CLEAR` read failure (FireWire address `0xfffff000000c`).
    *   Attempt to read `GPCSR_CHIP_ID` (logical `0xc700 0014`) via FireWire once mapping is known.

**Phase 2: Planning & Implementation Strategy**

1.  **Address Read Reliability (Prioritized):**
    *   Resolve basic FireWire read issue (`CSR STATE_CLEAR` failure).
    *   Confirm/Determine correct FireWire base address for DICE registers.
2.  **Improve Read Efficiency (Block Reads):**
    *   Plan to use FireWire `ReadBlock` for contiguous register areas (Global, TX, RX parameter spaces) once basic reads are reliable.
3.  **Validate Data Interpretation:**
    *   Verify register definitions (`DiceDefines.hpp`) against datasheet.
    *   Confirm correct endianness handling.
    *   Verify logic for extracting stream counts, sizes, etc.
4.  **Remove Artificial Limits:**
    *   Remove hardcoded limits in `readDiceRegisters` after validation. Implement only essential sanity checks.
5.  **Refine Scanner Output:**
    *   Enhance `firewire_scanner.cpp` for clarity on read success/failure and interpreted values.

**Phase 3: Implementation & Testing (Likely in Code Mode)**

1.  Implement changes iteratively, prioritizing read reliability and address mapping.
2.  Test frequently with `firewire_scanner`.
3.  Refine scanner output.

**Mermaid Sequence Diagram (Illustrating Efficient Block Reads):**

```mermaid
sequenceDiagram
    participant S as firewire_scanner
    participant D as DiceAudioDevice
    participant IOKit as IOKit/IOFireWireDeviceInterface

    S->>D: scanDevices() / init()
    D->>D: initIoFunctions() / readDiceRegisters()
    Note over D: Identify contiguous register blocks (Global, TX, RX)
    D->>IOKit: ReadBlock(Global Params Address, Size)
    IOKit-->>D: Global Param Data / Status
    D->>D: Parse Global Params (Counts, Sizes, Offsets)
    Note over D: Validate parsed values (No artificial limits)
    D->>IOKit: ReadBlock(TX Params Address, Size)
    IOKit-->>D: TX Param Data / Status
    D->>D: Parse TX Params (Channels, MIDI, etc. for ALL streams)
    Note over D: Validate parsed values
    D->>IOKit: ReadBlock(RX Params Address, Size)
    IOKit-->>D: RX Param Data / Status
    D->>D: Parse RX Params (Channels, MIDI, etc. for ALL streams)
    Note over D: Validate parsed values
    D-->>S: Device Info (with validated, complete data)
    S->>S: Print Detailed Device Info
