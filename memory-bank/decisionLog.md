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
---
[2025-05-04 01:09:00] - Decision: Fix format on save functionality for C/C++ files.
[2025-05-04 01:09:00] - Rationale: Format on save was not working despite being enabled in settings. Investigation revealed that clang-format was not installed on the system and there were potential conflicts with other formatters.
[2025-05-04 01:09:00] - Implementation Details: 1) Installed clang-format via Homebrew; 2) Updated VSCode settings to explicitly point to the clang-format executable at /opt/homebrew/bin/clang-format; 3) Explicitly set ms-vscode.cpptools as the default formatter for C/C++ files; 4) Disabled Prettier for C/C++ files to avoid conflicts; 5) Enabled "Fix All" code actions on save for additional formatting.
[2025-05-04 01:09:00] - Implications: Developers will now have consistent code formatting across the project, improving code readability and maintainability. The .clang-format file in the project root will be used to ensure all code follows the project's formatting standards.
[2025-05-03 15:32:06] - Implications: The scanner now correctly reports unique channel names without duplicates, providing a more accurate representation of the device's channel configuration. This will be important for the DICE driver implementation, which will rely on this information for audio routing.
[2025-05-04 01:11:30] - Decision: Update .clang-format to place commas on the left.
[2025-05-04 01:11:30] - Rationale: User requested that commas should be on the left when using auto linting. This is a specific formatting preference that needed to be configured in the .clang-format file.
[2025-05-04 01:11:30] - Implementation Details: Added the following settings to .clang-format: 1) BinPackArguments: false and BinPackParameters: false to prevent arguments and parameters from being packed onto a single line; 2) BreakBeforeInheritanceComma: true and BreakConstructorInitializersBeforeComma: true to place commas at the beginning of lines in inheritance lists and constructor initializers; 3) BreakBeforeBinaryOperators: All to place operators at the beginning of lines.
[2025-05-04 01:11:30] - Implications: Code formatting will now place commas on the left, which is the preferred style for this project. This will ensure consistent formatting across all files.
[2025-05-04 01:12:40] - Decision: Update .clang-format to disable alignment of consecutive assignments and declarations.
[2025-05-04 01:12:40] - Rationale: User requested not to align every `=` on each line. This is a specific formatting preference that needed to be configured in the .clang-format file.
[2025-05-04 01:12:40] - Implementation Details: Changed `AlignConsecutiveAssignments` and `AlignConsecutiveDeclarations` from `true` to `false` in the .clang-format file.
[2025-05-04 01:12:40] - Implications: Code formatting will no longer align consecutive assignments (`=`) and declarations, which is the preferred style for this project. This will ensure consistent formatting across all files.
[2025-05-04 01:16:15] - Decision: Update .clang-format to achieve specific function argument formatting style.
[2025-05-04 01:16:15] - Rationale: User provided a specific example of how they want function arguments to be formatted, with each argument on its own line and significantly indented.
[2025-05-04 01:16:15] - Implementation Details: Added the following settings to .clang-format: 1) `AllowAllArgumentsOnNextLine: false` to prevent all arguments from being placed on the next line; 2) `AllowAllParametersOfDeclarationOnNextLine: false` to prevent all parameters from being placed on the next line; 3) `ContinuationIndentWidth: 60` to set a large indentation for continued lines.
[2025-05-04 01:16:15] - Implications: Function arguments will now be formatted with each argument on its own line and significantly indented, matching the user's preferred style.
[2025-05-04 01:17:45] - Decision: Update formatting settings to preserve whitespace and indentation.
[2025-05-04 01:17:45] - Rationale: User requested not to trim spaces on newlines and to include the same amount of space as the indent level.
[2025-05-04 01:17:45] - Implementation Details: 1) Added `files.insertFinalNewline: true` and `files.trimFinalNewlines: false` to VSCode settings; 2) Added `KeepEmptyLinesAtTheStartOfBlocks: true` and `MaxEmptyLinesToKeep: 2` to .clang-format.
[2025-05-04 01:17:45] - Implications: Code formatting will now preserve whitespace and indentation, which is the preferred style for this project. This will ensure consistent formatting across all files.

[2025-05-04 01:20:05] - Decision: Update .clang-format to match the provided example code style.
[2025-05-04 01:20:05] - Rationale: User provided a complete example of their preferred formatting style, which required additional adjustments to the .clang-format file.
[2025-05-04 01:20:05] - Implementation Details: 1) Changed BasedOnStyle from Google to LLVM; 2) Added spacing settings to match the example, including SpacesBeforeTrailingComments, SpacesInParentheses, SpacesInSquareBrackets, SpacesInAngles, SpaceInEmptyParentheses, SpaceBeforeParens, SpaceBeforeRangeBasedForLoopColon, SpaceBeforeInheritanceColon, and SpaceBeforeAssignmentOperators.
[2025-05-04 01:20:05] - Implications: Code formatting will now more closely match the example provided by the user, ensuring consistent formatting across all files.
