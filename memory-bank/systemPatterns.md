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
*   **DICE Hardware Interaction:** Three approaches for register access:
    1. Standard Base Method (Current): Uses fixed base address (0xffffe0000000) with predefined offsets for Global (base+0), TX (base+0x400), and RX (base+0x800) spaces. This matches FFADO's approach and provides reliable access to core functionality.
    2. Dynamic Discovery Method (Future): Device exposes 64-bit pointers through Config ROM key 0xD1 (e.g., Global: 0x10000170d000081, TX: 0xc087000c010000d1). These pointers include metadata in lower bits and may enable advanced features.
    3. EAP Space Access (Extended): Uses base+0x200000 for Extended Application Protocol space, with structured sections:
       - Capability (0x0000): Device capabilities and features
       - Command (0x0008): Command interface
       - Mixer (0x0010): Audio mixing controls
       - Peak (0x0018): Peak metering
       - New Routing (0x0020): Enhanced routing configuration
       - Stream Config (0x0028): Stream settings
       - Current Config (0x0030): Active configuration
       - Standalone Config (0x0038): Offline settings
       - App Space (0x0040): Application-specific data
       Each section uses standardized offsets and may contain ASCII strings for identification.
    
    Quadlet read/write transactions are used for register access. Block reads are used for extended spaces. Asynchronous updates from the device are handled via ARM to a host-provided Notification Register address.
*   **AVS Streaming (IEC 61883-6):** FireWire isochronous audio/MIDI streaming uses the AM824 format, embedding timing (SYT field in CIP header), channel status, user bits, and MIDI data within the audio stream quadlets.
*   **Userspace FireWire Driver (libraw1394):** Direct interaction with the FireWire bus from userspace using libraries like `libraw1394` (as seen in FFADO). Involves manual quadlet reads/writes, isochronous stream setup, and ARM handling for notifications.
*   (Other patterns to be documented)

## Testing Patterns

*   (To be documented)

[2025-05-06 00:21:00] - Refined DICE pattern and added Userspace Driver pattern based on FFADO analysis.
[2025-05-06 00:00:00] - Added DICE Hardware Interaction and AVS Streaming patterns based on documentation review.

---
[2025-0

---
[2025-05-06 17:43:00] - Conditional EAP Support Pattern
## Architectural Patterns
- **Conditional Feature Initialization:** Introduced a pattern for conditionally enabling/disabling device features based on known compatibility issues.
    - **Mechanism:** Uses a blacklist (`std::set<DeviceIdentifier>`) defined in `DiceDefines.hpp` containing Vendor/Model IDs of devices with known issues (e.g., EAP incompatibility).
    - **Implementation:** During `DiceAudioDevice::init()`, the device's identifier is checked against the blacklist. A boolean flag (`m_supportsEAP`) is set accordingly.
    - **Usage:** EAP-related initialization (`DiceEAP::init()`) and access (`DiceAudioDevice::getEAP()`) are guarded by this flag. If initialization fails even for a non-blacklisted device, the flag is set to false for the session.
    - **Benefit:** Allows maintaining potentially useful code (like EAP) while preventing it from running on devices where it's known to cause problems, improving stability and compatibility.
