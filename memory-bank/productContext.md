# Product Context

This file provides a high-level overview of the project and the expected product that will be created. Initially it is based upon projectBrief.md (if provided) and all other available project-related information in the working directory. This file is intended to be updated as the project evolves, and should be used to inform all other modes of the project's goals and context.
YYYY-MM-DD HH:MM:SS - Log of updates made will be appended as footnotes to the end of this file.

*

## Project Goal

*   Re-implement FireWire audio functionality on macOS.
*   Address issues with DICE chipset support.

## Key Features

*   FireWire device discovery and management.
*   Audio streaming via isochronous transfer.
*   Control application (FWA-Control) for interaction.
*   Kernel driver for low-level access.

## Overall Architecture

*   macOS Driver (`src/driver`)
*   Core Logic Library (`src/FWA`, `src/Isoch`)
*   XPC Daemon (`src/xpc/FWADaemon`)
*   SwiftUI Control App (`FWA-Control`)
*   Debugging Tools (`src/tools`)

## DICE Chipset and Memory Map Details (from Datasheet)

Based on the `docs/dice-datasheet.pdf`, the following key details about the DICE chipsets and their memory maps are relevant to the project:

### DICE Chipsets

*   **DICE TCD2210 (DICE Mini):** Reduced version with one audio port, available in QFP 128 package.
*   **DICE TCD2220 (DICE JR):** Full version with dual audio port, available in LQFP 144 package.

### Major Functional Blocks and Base Memory Addresses

*   **ARM7TDMI:** 32-bit RISC processor core.
*   **External Bus Interface (EBI):** Memory interface (Base Address: 0x8100_0000).
*   **1394 Link Layer Controller (LLC):** IEEE 1394a compliant (Base Address: 0x8200_0000).
*   **UART:** Dual serial interface (UART#0 Base Address: 0xBE00_0000, UART#1 Base Address: 0xBD00_0000).
*   **Watch Dog:** Counter for system reset or timer (Base Address: 0xBF00_0000).
*   **Address Remap:** Handles boot and normal memory maps (Base Address: 0xC000_0000).
*   **Interrupt Controller:** Handles IRQ and FIQ sources (Base Address: 0xC100_0000).
*   **Dual Timer:** Two programmable 32-bit timers (Base Address: 0xC200_0000).
*   **GPIO:** General Purpose I/O (Base Address: 0xC300_0000).
*   **I2C:** Two-wire serial interface (Base Address: 0xC400_0000).
*   **SPI:** Serial Peripheral Interface (Base Address: 0xC500_0000).
*   **GRAY:** Rotary Encoder Interface (Base Address: 0xC600_0000).
*   **GPCSR:** General Purpose Control and Status Registers (Base Address: 0xC700_0000).
*   **Jet PLL:** Jitter Elimination Technologies PLL (Base Address: 0xCC00_0000).
*   **DICE Router:** Handles audio transfers (Base Address: 0xCE00_0000).
*   **Clock Controller:** Handles clock selection and sync (Memory Map starts at 0xce01 0000 within DICE Sub System).
*   **AES Receivers:** 4 AES/EBU Receivers (Memory Map starts at 0xce02 0000 within DICE Sub System).
*   **AES Transmitters:** 4 AES/EBU Transmitters (Memory Map starts at 0xce03 0000 within DICE Sub System).
*   **InS Transmitters:** Configurable serial audio interface transmitters (Memory Map starts at 0xce09 0000 (InS0), 0xce0b 0000 (InS1) within DICE Sub System).
*   **InS Receivers:** Configurable serial audio interface receivers (Memory Map starts at 0xce08 0000 (InS0), 0xce0a 0000 (InS1) within DICE Sub System).
*   **ADAT Receiver:** 2 ADAT compatible receivers (Memory Map starts at 0xce04 0000 within DICE Sub System).
*   **ADAT Transmitter:** 2 ADAT compatible transmitters (Memory Map starts at 0xce05 0000 within DICE Sub System).
*   **ARM Audio Transceiver:** ARM processor audio access (Memory Map starts at 0xce16 0000 within DICE Sub System).
*   **Audio Mixer:** Creates audio sub-mixes (Memory Map starts at 0xce06 0000 within DICE Sub System).
*   **AVS (Audio Video System):** Handles isochronous streaming (Base Address: 0xCF00_0000).
*   **AVS Audio Receivers:** 2 audio receivers (Memory Map starts at 0xcf00 0000 within AVS Sub System).
*   **AVS Audio Transmitters:** 2 audio transmitters (Memory Map starts at 0xcf00 00c0 within AVS Sub System).
*   **AVS ITP:** Internal Time Processor (Memory Map starts at 0xcf00 01f8 within AVS Sub System).
*   **AVS Audio Transmitter Format Handler:** Handles IEC 60958 transmission (Memory Map starts at 0xcf00 02c0 (FMT_TXDI1), 0xcf00 0340 (FMT_TXDI2) within AVS Sub System).
*   **AVS Audio Receiver Format Handler:** Handles IEC 60958 reception (Memory Map starts at 0xcf00 0200 (FORMAT_RXDI1), 0xcf00 0230 (FORMAT_RXDI2) within AVS Sub System).
*   **AVS Interrupt Controller:** Gathers AVS interrupts (Memory Map starts at 0xcf00 013c within AVS Sub System).
*   **AVS Media FIFO:** Handles Isoc. Stream data buffering (Memory Map starts at 0xcf00 0184 within AVS Sub System).
*   **AVS MIDI Interface:** Handles MIDI data (Memory Map starts at 0xcf00 01e4 within AVS Sub System).
*   **Power Manager:** Handles power management (Base Address: 0xD000_0000).

[2025-05-03 16:11:30] - Added DICE Chipset and Memory Map details from datasheet.