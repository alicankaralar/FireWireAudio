#ifndef CHANNEL_COUNT_VALIDATOR_HPP
#define CHANNEL_COUNT_VALIDATOR_HPP

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Structure to hold channel counts from different sources for
 * cross-validation
 */
struct ChannelCounts {
  // Source A: Counts from channel names
  int namesMonoOutputs = 0;
  int namesMonoInputs = 0;
  int namesStereoOutputPairs = 0;
  int namesStereoInputPairs = 0;
  int namesTotalPhysicalOutputs = 0;
  int namesTotalPhysicalInputs = 0;

  // Source B: Counts from stream registers
  int streamsAudioOutputs = 0;
  int streamsMidiOutputs = 0;
  int streamsAudioInputs = 0;
  int streamsMidiInputs = 0;
  int streamsTotalOutputs = 0;
  int streamsTotalInputs = 0;

  // Source C: Counts from EAP
  int eapAudioOutputs = 0;
  int eapMidiOutputs = 0;
  int eapAudioInputs = 0;
  int eapMidiInputs = 0;
  int eapTotalOutputs = 0;
  int eapTotalInputs = 0;

  // Final counts (after prioritization)
  int finalTotalOutputs = 0;
  int finalTotalInputs = 0;

  // Validation status
  bool hasDiscrepancy = false;
};

/**
 * @brief Calculates channel counts from stream registers
 *
 * Sums up NB_AUDIO and NB_MIDI values across all TX and RX streams
 *
 * @param device The FireWireDevice containing register values
 * @param counts The ChannelCounts structure to update with stream counts
 */
void calculateStreamChannelCounts(const FireWireDevice &device,
                                  ChannelCounts &counts);

/**
 * @brief Calculates channel counts from EAP data
 *
 * Extracts audio and MIDI channel counts from EAP configuration
 *
 * @param device The FireWireDevice containing EAP data
 * @param counts The ChannelCounts structure to update with EAP counts
 */
void calculateEAPChannelCounts(const FireWireDevice &device,
                               ChannelCounts &counts);

/**
 * @brief Cross-validates channel counts from different sources
 *
 * Compares counts from channel names, stream registers, and EAP data
 * Logs discrepancies and determines final counts based on priority
 *
 * @param counts The ChannelCounts structure containing counts from all sources
 * @return true if validation was successful, false if critical discrepancies
 * were found
 */
bool crossValidateChannelCounts(ChannelCounts &counts);

/**
 * @brief Prints a summary of channel counts from all sources
 *
 * @param counts The ChannelCounts structure containing counts from all sources
 */
void printChannelCountSummary(const ChannelCounts &counts);

} // namespace FWA::SCANNER

#endif // CHANNEL_COUNT_VALIDATOR_HPP
