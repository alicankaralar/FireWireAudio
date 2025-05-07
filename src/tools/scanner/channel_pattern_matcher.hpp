#ifndef CHANNEL_PATTERN_MATCHER_HPP
#define CHANNEL_PATTERN_MATCHER_HPP

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "string_extraction.hpp" // For StringMatch struct

namespace FWA::SCANNER {
/**
 * @brief Structure to hold the results of channel pattern matching
 */
struct ChannelPatternResults {
  // Output channels
  std::set<std::string> outputChannelNames;
  std::set<int> outputChannelNumbers;

  // Input channels
  std::set<std::string> inputChannelNames;
  std::set<int> inputChannelNumbers;

  // Stereo output channels
  std::set<std::string> stereoOutputChannels;
  std::set<int> stereoOutputChannelNumbers;

  // Stereo input channels
  std::set<std::string> stereoInputChannels;
  std::set<int> stereoInputChannelNumbers;
};

/**
 * @brief Finds output and input channel patterns in the extracted strings
 *
 * Searches for patterns like "OUTPUT CH1", "INPUT CH2", etc. in the
 * extracted strings and categorizes them.
 *
 * @param strings Vector of StringMatch objects containing the extracted strings
 * @param results ChannelPatternResults structure to store the results
 */
void findChannelPatterns(const std::vector<StringMatch> &strings,
                         ChannelPatternResults &results);

/**
 * @brief Finds stereo channel patterns in the extracted strings
 *
 * Searches for patterns like "OUTPUT ST1L", "INPUT ST2R", etc. in the
 * extracted strings and categorizes them.
 *
 * @param strings Vector of StringMatch objects containing the extracted strings
 * @param results ChannelPatternResults structure to store the results
 */
void findStereoChannelPatterns(const std::vector<StringMatch> &strings,
                               ChannelPatternResults &results);

/**
 * @brief Analyzes extracted strings to find all channel patterns
 *
 * Combines the results of findChannelPatterns and findStereoChannelPatterns
 * to provide a complete analysis of channel patterns.
 *
 * @param strings Vector of StringMatch objects containing the extracted strings
 * @return ChannelPatternResults structure containing the analysis results
 */
ChannelPatternResults
analyzeChannelPatterns(const std::vector<StringMatch> &strings);

/**
 * @brief Reports the results of channel pattern analysis
 *
 * Prints a detailed report of the channel patterns found, including
 * counts of different channel types.
 *
 * @param results ChannelPatternResults structure containing the analysis
 * results
 */
void reportChannelPatterns(const ChannelPatternResults &results);

} // namespace FWA::SCANNER

#endif // CHANNEL_PATTERN_MATCHER_HPP
