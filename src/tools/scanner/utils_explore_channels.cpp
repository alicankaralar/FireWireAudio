#include "utils_explore_channels.hpp"
#include "channel_name_extractor.hpp" // For extractChannelNames
#include "channel_pattern_matcher.hpp" // For analyzeChannelPatterns, reportChannelPatterns
#include "channel_summary_reporter.hpp" // For processChannelInformation
#include "scanner.hpp"                  // For FireWireDevice

#include <iostream>

namespace FWA::SCANNER {

void exploreChannelNamesArea(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt32 generation) {
  std::cout << "\n=== DETAILED CHANNEL NAMES EXPLORATION ===\n" << std::endl;

  // Step 1: Extract channel names from device memory
  std::vector<StringMatch> channelStrings =
      extractChannelNames(deviceInterface, service, device, generation);

  if (channelStrings.empty()) {
    std::cout << "No channel names could be extracted." << std::endl;
    std::cout << "\n=== END CHANNEL NAMES EXPLORATION ===\n" << std::endl;
    return;
  }

  // Step 2: Analyze channel patterns
  ChannelPatternResults patternResults = analyzeChannelPatterns(channelStrings);

  // Step 3: Report channel patterns
  reportChannelPatterns(patternResults);

  // Step 4: Process channel information (validation, counting, summary)
  processChannelInformation(device, patternResults);

  std::cout << "\n=== END CHANNEL NAMES EXPLORATION ===\n" << std::endl;
}

} // namespace FWA::SCANNER
