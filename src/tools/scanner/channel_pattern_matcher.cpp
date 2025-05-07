#include "channel_pattern_matcher.hpp"
#include "string_extraction.hpp" // For StringMatch struct

#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <vector>

namespace FWA::SCANNER {

void findChannelPatterns(const std::vector<StringMatch> &strings,
                         ChannelPatternResults &results) {
  // Advanced regex patterns for channel names with improved flexibility
  // Using non-capturing groups (?:...) for grouping without creating
  // backreferences Using character classes [\s\-_] to match various separators
  // (space, hyphen, underscore) Making parts of the pattern optional with ?
  // (e.g., (?:CH)? to match "CH" or nothing) Preserving the capturing group for
  // the channel number (\d+) Adding (?!\S) to ensure it's not part of a larger
  // word
  std::regex outputChannelPattern(
      R"((?:OUT|OUTPUT)[\s\-_]*(?:CH)?(\d+)(?!\S))");
  std::regex inputChannelPattern(R"((?:IN|INPUT)[\s\-_]*(?:CH)?(\d+)(?!\S))");

  // Additional flexible patterns for validation (not used for counting
  // directly) Enhanced with non-capturing groups and more flexible separators
  std::regex flexibleOutputPattern(
      R"((?:OUT|OUTPUT|ANALOG\s+OUT|LINE\s+OUT|ADAT\s+OUT|SPDIF\s+OUT)[\s\-_]*(\d+)(?!\S))");
  std::regex flexibleInputPattern(
      R"((?:IN|INPUT|ANALOG\s+IN|LINE\s+IN|MIC|ADAT\s+IN|SPDIF\s+IN)[\s\-_]*(\d+)(?!\S))");

  // Combine all string texts for pattern matching
  std::string allText;
  for (const auto &match : strings) {
    allText += match.text + " "; // Add space to separate strings
  }

  // Now search for patterns in the combined text
  std::smatch match;
  std::string::const_iterator searchStart(allText.cbegin());

  // Find all output channels
  while (std::regex_search(searchStart, allText.cend(), match,
                           outputChannelPattern)) {
    results.outputChannelNames.insert(
        match[0]); // Using insert instead of push_back
    // Extract the channel number
    int channelNum = std::stoi(match[1]);
    results.outputChannelNumbers.insert(channelNum);
    searchStart = match.suffix().first;
  }

  // Find all input channels
  searchStart = allText.cbegin();
  while (std::regex_search(searchStart, allText.cend(), match,
                           inputChannelPattern)) {
    results.inputChannelNames.insert(
        match[0]); // Using insert instead of push_back
    // Extract the channel number
    int channelNum = std::stoi(match[1]);
    results.inputChannelNumbers.insert(channelNum);
    searchStart = match.suffix().first;
  }
}

void findStereoChannelPatterns(const std::vector<StringMatch> &strings,
                               ChannelPatternResults &results) {
  // Advanced regex patterns for stereo channels with improved flexibility
  // Using non-capturing groups and character classes for better matching
  std::regex stereoOutputPattern(
      R"((?:OUT|OUTPUT)[\s\-_]*(?:ST|STEREO)[\s\-_]*(?:CH)?(\d+)([LR])(?!\S))");
  std::regex stereoInputPattern(
      R"((?:IN|INPUT)[\s\-_]*(?:ST|STEREO)[\s\-_]*(?:CH)?(\d+)([LR])(?!\S))");

  // Additional flexible patterns for stereo validation
  // Enhanced with non-capturing groups and more flexible separators
  std::regex flexibleStereoOutputPattern(
      R"((?:OUT|OUTPUT|ANALOG\s+OUT|LINE\s+OUT|ADAT\s+OUT|SPDIF\s+OUT)[\s\-_]*(?:ST|STEREO)[\s\-_]*(\d+)[\s\-_]*([LR])(?!\S))");
  std::regex flexibleStereoInputPattern(
      R"((?:IN|INPUT|ANALOG\s+IN|LINE\s+IN|MIC|ADAT\s+IN|SPDIF\s+IN)[\s\-_]*(?:ST|STEREO)[\s\-_]*(\d+)[\s\-_]*([LR])(?!\S))");

  // Combine all string texts for pattern matching
  std::string allText;
  for (const auto &match : strings) {
    allText += match.text + " "; // Add space to separate strings
  }

  // Now search for patterns in the combined text
  std::smatch match;
  std::string::const_iterator searchStart(allText.cbegin());

  // Find all stereo output channels
  searchStart = allText.cbegin();
  while (std::regex_search(searchStart, allText.cend(), match,
                           stereoOutputPattern)) {
    results.stereoOutputChannels.insert(
        match[0]); // Using insert instead of push_back
    // Extract the channel number
    int channelNum = std::stoi(match[1]);
    results.stereoOutputChannelNumbers.insert(channelNum);
    searchStart = match.suffix().first;
  }

  // Find all stereo input channels
  searchStart = allText.cbegin();
  while (std::regex_search(searchStart, allText.cend(), match,
                           stereoInputPattern)) {
    results.stereoInputChannels.insert(
        match[0]); // Using insert instead of push_back
    // Extract the channel number
    int channelNum = std::stoi(match[1]);
    results.stereoInputChannelNumbers.insert(channelNum);
    searchStart = match.suffix().first;
  }
}

ChannelPatternResults
analyzeChannelPatterns(const std::vector<StringMatch> &strings) {
  ChannelPatternResults results;

  // Find regular channel patterns
  findChannelPatterns(strings, results);

  // Find stereo channel patterns
  findStereoChannelPatterns(strings, results);

  return results;
}

void reportChannelPatterns(const ChannelPatternResults &results) {
  std::cout << "\n--- Channel Name Patterns ---" << std::endl;

  // Report output channels
  if (!results.outputChannelNames.empty()) {
    std::cout << "Found " << results.outputChannelNames.size()
              << " output channel names matching pattern 'OUTPUT CH#':"
              << std::endl;
    for (const auto &name : results.outputChannelNames) {
      std::cout << "  " << name << std::endl;
    }
  } else {
    std::cout << "No output channel names matching pattern 'OUTPUT CH#' found."
              << std::endl;
  }

  // Report input channels
  if (!results.inputChannelNames.empty()) {
    std::cout << "Found " << results.inputChannelNames.size()
              << " input channel names matching pattern 'INPUT CH#':"
              << std::endl;
    for (const auto &name : results.inputChannelNames) {
      std::cout << "  " << name << std::endl;
    }
  } else {
    std::cout << "No input channel names matching pattern 'INPUT CH#' found."
              << std::endl;
  }

  // Report stereo output channels
  if (!results.stereoOutputChannels.empty()) {
    std::cout << "Found " << results.stereoOutputChannels.size()
              << " stereo output channel names:" << std::endl;
    for (const auto &name : results.stereoOutputChannels) {
      std::cout << "  " << name << std::endl;
    }
  }

  // Report stereo input channels
  if (!results.stereoInputChannels.empty()) {
    std::cout << "Found " << results.stereoInputChannels.size()
              << " stereo input channel names:" << std::endl;
    for (const auto &name : results.stereoInputChannels) {
      std::cout << "  " << name << std::endl;
    }
  }
}

} // namespace FWA::SCANNER
