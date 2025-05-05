# System Patterns *Optional*

This file documents recurring patterns and standards used in the project.
It is optional, but recommended to be updated as the project evolves.
YYYY-MM-DD HH:MM:SS - Log of updates made.

*

## Coding Patterns

*   (To be documented)

## Architectural Patterns

*   Driver/Daemon/App communication via XPC.
*   Use of C++ for core logic and driver, Swift/SwiftUI for the control application.
*   **DICE Hardware Interaction:** Communication relies on reading/writing specific 32-bit registers at offsets from a device-specific base address (often `0xFFFFF0000000` + offset, discovered via Config ROM key `0xD1`). Quadlet read/write transactions are used. Asynchronous updates from the device are handled via Asynchronous Request Mapping (ARM) to a host-provided Notification Register address.
*   **AVS Streaming (IEC 61883-6):** FireWire isochronous audio/MIDI streaming uses the AM824 format, embedding timing (SYT field in CIP header), channel status, user bits, and MIDI data within the audio stream quadlets.
*   **Userspace FireWire Driver (libraw1394):** Direct interaction with the FireWire bus from userspace using libraries like `libraw1394` (as seen in FFADO). Involves manual quadlet reads/writes, isochronous stream setup, and ARM handling for notifications.
*   (Other patterns to be documented)

## Testing Patterns

*   (To be documented)

[2025-05-06 00:21:00] - Refined DICE pattern and added Userspace Driver pattern based on FFADO analysis.
[2025-05-06 00:00:00] - Added DICE Hardware Interaction and AVS Streaming patterns based on documentation review.
