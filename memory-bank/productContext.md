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
*   **DICE Sub System:** Contains audio processing blocks (Base Address: 0xCE00_0000).
    *   **DICE Router:** Handles audio transfers (Offset: 0x0000).
    *   **Clock Controller:** Handles clock selection and sync (Offset: 0x0100).
    *   **AES Receivers:** 4 AES/EBU Receivers (Offset: 0x0200).
    *   **AES Transmitters:** 4 AES/EBU Transmitters (Offset: 0x0300).
    *   **ADAT Receiver:** 2 ADAT compatible receivers (Offset: 0x0400).
    *   **ADAT Transmitter:** 2 ADAT compatible transmitters (Offset: 0x0500).
    *   **Audio Mixer:** Creates audio sub-mixes (Offset: 0x0600).
    *   **InS Receivers:** Configurable serial audio interface receivers (Offset: 0x0800 (InS0), 0x0A00 (InS1)).
    *   **InS Transmitters:** Configurable serial audio interface transmitters (Offset: 0x0900 (InS0), 0x0B00 (InS1)).
    *   **ARM Audio Transceiver:** ARM processor audio access (Offset: 0x1600).
*   **AVS (Audio Video System):** Handles isochronous streaming (Base Address: 0xCF00_0000).
    *   **AVS Audio Receivers:** 2 audio receivers (Offset: 0x0000).
    *   **AVS Audio Transmitters:** 2 audio transmitters (Offset: 0x00C0).
    *   **AVS Audio Receiver Format Handler:** Handles IEC 60958 reception (Offset: 0x0200 (FORMAT_RXDI1), 0x0230 (FORMAT_RXDI2)).
    *   **AVS Audio Transmitter Format Handler:** Handles IEC 60958 transmission (Offset: 0x02C0 (FMT_TXDI1), 0x0340 (FMT_TXDI2)).
    *   **AVS Interrupt Controller:** Gathers AVS interrupts (Offset: 0x013C).
    *   **AVS Media FIFO:** Handles Isoc. Stream data buffering (Offset: 0x0184).
    *   **AVS MIDI Interface:** Handles MIDI data (Offset: 0x01E4).
    *   **AVS ITP:** Internal Time Processor (Offset: 0x01F8).
*   **Power Manager:** Handles power management (Base Address: 0xD000_0000).

### DICE Protocol Overview (from Specification)

*   **Communication:** Primarily via reading/writing specific registers within the DICE address space (typically `0xFFFFF0000000` + offset).
*   **Key Register Areas:**
    *   Global Registers (Device capabilities, clock settings, nickname).
    *   Owner Registers (Notification settings, acquisition status).
    *   Tx/Rx Stream Registers (Configuration for each audio stream).
    *   Routing Table (Internal audio connections).
*   **Notifications:** Devices can notify the host of changes (e.g., clock source, stream parameters) via asynchronous FireWire transactions to a host-specified Notification Register address.
*   **Clocking:** Supports various sources (Internal, Word Clock, ADAT, AES, FireWire). JetPLL™ technology provides low-jitter clocking.
*   **Audio Interfaces:** Supports I2S, AES/EBU, ADAT (including S/MUX).
*   **AVS Streaming:** Uses IEC 61883-6 (AM824 format) for FireWire audio/MIDI streaming, including CIP headers and SYT timestamps.

[2025-05-03 16:11:30] - Added DICE Chipset and Memory Map details from datasheet.
[2025-05-05 23:59:00] - Enhanced Memory Map with DICE/AVS sub-module offsets and added DICE Protocol Overview based on specification documents.
## Linux DICE Driver (`ref/dice.c`) Insights

Analysis of the provided Linux kernel driver (`ref/dice.c`) reveals valuable information about DICE device support:

*   **Supported Vendors/OUIs:** The driver explicitly handles devices from:
    *   Weiss (0x001c6a)
    *   Loud (0x000ff2)
    *   Focusrite (0x00130e)
    *   TC Electronic (0x000166)
    *   Alesis (0x000595)
    *   M-Audio (0x000d6c)
    *   Mytek (0x001ee8)
    *   SSL (0x0050c2)
    *   Presonus (0x000a92)
    *   Harman (0x000fd7)
    *   Avid (0x00a07e)
*   **Device Identification:** Uses a combination of Vendor ID (Specifier ID), Model ID, and Category ID (from Config ROM) to identify devices.
*   **Device-Specific Logic:** Contains a table (`dice_id_table`) mapping specific Vendor/Model IDs to custom functions, primarily for detecting audio stream formats (e.g., `snd_dice_detect_tcelectronic_formats`, `snd_dice_detect_alesis_formats`, `snd_dice_detect_extension_formats`, `snd_dice_detect_mytek_formats`, `snd_dice_detect_presonus_formats`, `snd_dice_detect_harman_formats`, `snd_dice_detect_focusrite_pro40_tcd3070_formats`, `snd_dice_detect_weiss_formats`).
*   **Known Quirks:** Includes specific workarounds or flags for certain devices, such as disabling double PCM frames (`disable_double_pcm_frames = true`) for some M-Audio/Avid models and handling GUID inconsistencies for Focusrite Saffire Pro 40.

This information provides a reference point for understanding how these devices are handled in another OS environment and highlights potential complexities or device variations to consider in this project's implementation.

### libhitaki (Linux Userspace Library) Insights

*   **Purpose:** A GLib/GObject-based library for interacting with ALSA FireWire kernel drivers (`snd-firewire-lib`).
*   **Communication:** Uses the ALSA HwDep character device (`/dev/snd/hwC*D*`) interface, relying on ioctl calls for synchronous operations (locking, getting info) and reading the device file descriptor for asynchronous events (e.g., `SNDRV_FIREWIRE_EVENT_DICE_NOTIFICATION`).
*   **Relevance:** Provides an alternative implementation example (userspace + kernel driver via ioctl/events) compared to the direct IOKit approach used in this project. Highlights the event-based notification pattern used in Linux.

### FFADO (Linux Userspace Library) Insights (`ref/libffado-2.4.9`)

Analysis of the FFADO library source code provides another perspective on DICE interaction:

*   **Architecture:** FFADO is a C++ userspace library using `libraw1394` for direct FireWire communication (quadlet reads/writes, isochronous stream management, ARM notifications).
*   **Register Access:**
    *   FFADO dynamically determines the base addresses for the Global, Tx, and Rx register spaces by reading specific registers (`DICE_REGISTER_GLOBAL_PARAMETER_SPACE_OFFSET`, etc.) relative to a base pointer (`DICE_REGISTER_BASE = 0x0000FFFFE0000000ULL`).
    *   It then uses relative offsets defined in `dice_defines.h` to access registers *within* these dynamically located spaces.
*   **Register Offset Discrepancies (Spec vs. FFADO):** This dynamic base + relative offset approach leads to different effective offsets compared to the absolute offsets defined in the DICE specification PDF (relative to `0xFFFFF0000000`). Key examples:
    *   **GLOBAL_OWNER:** Spec: `0x04` | FFADO: `0x00` (relative to dynamic Global base)
    *   **GLOBAL_NOTIFICATION:** Spec: `0x88` (Owner space) | FFADO: `0x08` (relative to dynamic Global base)
    *   **GLOBAL_CLOCK_SELECT:** Spec: `0x20` | FFADO: `0x4C` (relative to dynamic Global base)
    *   **TX_ISOCHRONOUS_CHANNEL (Base):** Spec: `0x10C` | FFADO: `0x08` (relative to dynamic Tx base)
    *   **RX_ISOCHRONOUS_CHANNEL (Base):** Spec: `0x20C` | FFADO: `0x08` (relative to dynamic Rx base)
    *   **Note:** This difference is crucial and suggests relying solely on the specification's absolute offsets might be incorrect if the device expects interaction relative to dynamically discovered bases, as implemented in FFADO.
*   **Notifications:** Uses ARM handlers registered via `libraw1394`. The host provides a notification address via `GLOBAL_OWNER`, and the device writes notification codes there.
*   **Streaming:** Uses `AmdtpReceiveStreamProcessor` / `AmdtpTransmitStreamProcessor` (IEC 61883-6 AM824). Configures streams by writing the allocated ISO channel number to DICE Tx/Rx registers.
*   **Ownership:** Uses compare-and-swap on `GLOBAL_OWNER` via `libraw1394`'s `lockCompareSwap64` to claim ownership and register the ARM handler. Includes checks for conflicts with the `snd-dice` kernel driver.
*   **EAP Support:** Includes an `EAP` class for Extended Audio Protocol support.
*   **Device Specialization:** Uses C++ inheritance (`Dice::Device` base class) for device-specific implementations (e.g., `Focusrite::SaffirePro40`, `Maudio::Profire2626`).

[2025-05-06 00:19:00] - Added FFADO insights, including register offset comparison.
[2025-05-06 00:12:00] - Refined Linux Driver Insights and added libhitaki insights.
