# Decision Log

This file records architectural and implementation decisions using a list format.
YYYY-MM-DD HH:MM:SS - Log of updates made.

*

## Decision

*   Initialize Memory Bank to maintain project context.

## Rationale

*   Requested by the user to persist information across sessions.
*   Helps track progress, decisions, and context for a complex project.

## Implementation Details

*   Created standard Memory Bank files: `productContext.md`, `activeContext.md`, `progress.md`.
*   Will create `decisionLog.md` and `systemPatterns.md`.
---
[2025-05-03 06:32:26] - Decision: Modify device discovery and AudioDevice initialization to handle generic IOFireWireDevice services directly.
Rationale: AudioDevice::init() currently assumes the input service is an IOFireWireAVCUnit and traverses *up* the I/O Registry. This fails for DICE devices presenting only as IOFireWireDevice. The fix involves passing the correct IOFireWireDevice service from discovery to AudioDevice and removing the upward traversal logic in AudioDevice::init().
Implementation Details: Modify IOKitFireWireDeviceDiscovery::deviceAdded, AudioDevice constructor, and AudioDevice::init.
---
[2025-05-03 08:10:48] - Decision: Confirmed Midas Venice DICE register base address is 0xFFFFF0000000 via Config ROM key 0xD1.
[2025-05-03 08:10:48] - Rationale: Config ROM parsing logic in `firewire_scanner.cpp` successfully identified the vendor-specific key (0xD1) pointing to this base address.
[2025-05-03 08:10:48] - Implication: The primary blocker for reading DICE registers is not an incorrect base address, but the persistent `kIOReturnNotResponding` (-536838121) error when using `IOFireWireDeviceInterface::ReadQuadlet` in the 0xFFFFF0... address range. Further investigation must focus on this IOKit error.
---
[2025-05-03 14:56:44] - Decision: Integrate channel configuration information from scanner tool into the DICE driver implementation.
[2025-05-03 14:56:44] - Rationale: The scanner tool has successfully identified 64 I/O channels in the Venice device (24 mono output, 24 mono input, 4 stereo output pairs, 4 stereo input pairs). This information is critical for accurate device representation and proper audio routing.
[2025-05-03 14:56:44] - Implementation Details: Create new methods in DiceAudioDevice to read channel names from the alternate memory location (0xffffe00001a8), categorize channels by type, expose channel information through the API, and calculate channel counts. This approach avoids the kIOReturnNotResponding issue by using the alternate memory location where channel names are stored.
[2025-05-03 14:56:44] - Implications: The driver will have accurate information about channel configuration, enabling better user experience, improved routing capabilities, and compatibility with devices that have different channel configurations.
---
[2025-05-03 15:04:11] - Decision: Improve the firewire_scanner tool to make channel configuration discovery more dynamic and accurate.
[2025-05-03 15:04:11] - Rationale: The current scanner tool has hardcoded addresses for channel names, duplicated logic for string extraction, and issues with RX/TX register reading and channel count validation. These issues make it less reliable when scanning different DICE devices.
[2025-05-03 15:04:11] - Implementation Details: 1) Implement dynamic address discovery for channel names; 2) Create common utilities for string extraction; 3) Make stream count detection more dynamic with validation against hardware capabilities; 4) Improve regex patterns for channel name matching and add cross-validation between channel names and stream information.
[2025-05-03 15:04:11] - Implications: The improved scanner tool will be more robust across different DICE devices, provide more accurate information about channel configurations, and be more maintainable due to reduced code duplication. This will provide a solid foundation for the DICE driver implementation.
---
[2025-05-03 15:32:06] - Decision: Use sets instead of vectors for storing channel names in the scanner tool.
[2025-05-03 15:32:06] - Rationale: The scanner was finding duplicate channel names because it was extracting strings at both the quadlet level and byte level. Using sets automatically deduplicates the channel names, ensuring each channel is only reported once.
[2025-05-03 15:32:06] - Implementation Details: Modified the exploreChannelNamesArea function in utils.cpp to use std::set instead of std::vector for storing channel names. Also improved the calculation of total stereo channels to use the number of unique channel numbers rather than dividing the total number of stereo channel names by 2.
[2025-05-03 15:32:06] - Implications: The scanner now correctly reports unique channel names without duplicates, providing a more accurate representation of the device's channel configuration. This will be important for the DICE driver implementation, which will rely on this information for audio routing.