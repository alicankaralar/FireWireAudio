#include "channel_summary_reporter.hpp"
#include "channel_count_validator.hpp" // For ChannelCounts, cross-validation functions
#include "channel_pattern_matcher.hpp" // For ChannelPatternResults
#include "scanner.hpp"                 // For FireWireDevice
#include "validation_utils.hpp"        // For validateChannelNumbers

#include <iostream>

namespace FWA::SCANNER {

void calculateChannelCounts(const ChannelPatternResults &results,
                            ChannelCounts &counts) {
  // Calculate total channels
  int totalMonoOutputs = results.outputChannelNames.size();
  int totalMonoInputs = results.inputChannelNames.size();

  // For stereo channels, we count pairs (L/R) as one stereo channel
  // The number of stereo pairs is the number of unique channel numbers
  int totalStereoOutputs = results.stereoOutputChannelNumbers.size();
  int totalStereoInputs = results.stereoInputChannelNumbers.size();

  // Calculate total physical channels
  int totalPhysicalOutputs = totalMonoOutputs + (totalStereoOutputs * 2);
  int totalPhysicalInputs = totalMonoInputs + (totalStereoInputs * 2);

  // Store counts from Source A (Names) in the ChannelCounts structure
  counts.namesMonoOutputs = totalMonoOutputs;
  counts.namesMonoInputs = totalMonoInputs;
  counts.namesStereoOutputPairs = totalStereoOutputs;
  counts.namesStereoInputPairs = totalStereoInputs;
  counts.namesTotalPhysicalOutputs = totalPhysicalOutputs;
  counts.namesTotalPhysicalInputs = totalPhysicalInputs;
}

void generateChannelSummary(const ChannelCounts &counts) {
  std::cout << "\n--- Channel Configuration Summary ---" << std::endl;

  std::cout << "Output Channels:" << std::endl;
  std::cout << "  Mono: " << counts.namesMonoOutputs << std::endl;
  std::cout << "  Stereo Pairs: " << counts.namesStereoOutputPairs << " ("
            << counts.namesStereoOutputPairs * 2 << " channels)" << std::endl;
  std::cout << "  Total Physical Outputs: " << counts.namesTotalPhysicalOutputs
            << std::endl;

  std::cout << "Input Channels:" << std::endl;
  std::cout << "  Mono: " << counts.namesMonoInputs << std::endl;
  std::cout << "  Stereo Pairs: " << counts.namesStereoInputPairs << " ("
            << counts.namesStereoInputPairs * 2 << " channels)" << std::endl;
  std::cout << "  Total Physical Inputs: " << counts.namesTotalPhysicalInputs
            << std::endl;

  std::cout << "Total I/O Channels: "
            << (counts.namesTotalPhysicalOutputs +
                counts.namesTotalPhysicalInputs)
            << std::endl;
}

bool validateTotalChannelCount(const ChannelCounts &counts,
                               int maxReasonableChannels) {
  int totalChannels =
      counts.namesTotalPhysicalOutputs + counts.namesTotalPhysicalInputs;

  if (totalChannels > maxReasonableChannels) {
    std::cout << "Warning: Total channel count (" << totalChannels
              << ") exceeds reasonable limit of " << maxReasonableChannels
              << std::endl;
    return false;
  }

  return true;
}

void processChannelInformation(FireWireDevice &device,
                               const ChannelPatternResults &results) {
  // Validate channel numbers
  std::cout << "\n--- Channel Number Validation ---" << std::endl;
  validateChannelNumbers(results.outputChannelNumbers, "Output");
  validateChannelNumbers(results.inputChannelNumbers, "Input");
  validateChannelNumbers(results.stereoOutputChannelNumbers, "Stereo Output");
  validateChannelNumbers(results.stereoInputChannelNumbers, "Stereo Input");

  // Initialize channel counts structure for cross-validation
  ChannelCounts counts;

  // Calculate counts from pattern matching results
  calculateChannelCounts(results, counts);

  // Generate channel summary
  generateChannelSummary(counts);

  // Validate total channel count
  validateTotalChannelCount(counts);

  // Calculate counts from other sources
  calculateStreamChannelCounts(device, counts);
  calculateEAPChannelCounts(device, counts);

  // Cross-validate channel counts
  crossValidateChannelCounts(counts);

  // Print summary of all channel counts
  printChannelCountSummary(counts);
}

} // namespace FWA::SCANNER
