#ifndef CHANNEL_SUMMARY_REPORTER_HPP
#define CHANNEL_SUMMARY_REPORTER_HPP

#include <cstdint>
#include <string>

#include "channel_count_validator.hpp" // For ChannelCounts struct
#include "channel_pattern_matcher.hpp" // For ChannelPatternResults struct
#include "scanner.hpp"                 // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Calculates total channel counts from pattern matching results
 *
 * Calculates the total number of mono and stereo channels, and the
 * total number of physical channels.
 *
 * @param results ChannelPatternResults structure containing the pattern
 * matching results
 * @param counts ChannelCounts structure to store the calculated counts
 */
void calculateChannelCounts(const ChannelPatternResults &results,
                            ChannelCounts &counts);

/**
 * @brief Generates a summary of channel configuration
 *
 * Prints a detailed summary of the channel configuration, including
 * counts of different channel types and total I/O channels.
 *
 * @param counts ChannelCounts structure containing the channel counts
 */
void generateChannelSummary(const ChannelCounts &counts);

/**
 * @brief Validates the total channel count
 *
 * Checks if the total channel count is within reasonable limits.
 *
 * @param counts ChannelCounts structure containing the channel counts
 * @param maxReasonableChannels Maximum reasonable total channel count
 * @return true if the channel count is reasonable, false otherwise
 */
bool validateTotalChannelCount(const ChannelCounts &counts,
                               int maxReasonableChannels = 128);

/**
 * @brief Processes channel information and generates a complete report
 *
 * Combines the results of pattern matching, channel counting, and validation
 * to generate a complete report of the channel configuration.
 *
 * @param device The FireWireDevice containing device information
 * @param results ChannelPatternResults structure containing the pattern
 * matching results
 */
void processChannelInformation(FireWireDevice &device,
                               const ChannelPatternResults &results);

} // namespace FWA::SCANNER

#endif // CHANNEL_SUMMARY_REPORTER_HPP
