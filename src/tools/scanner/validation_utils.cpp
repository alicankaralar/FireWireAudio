#include "validation_utils.hpp"

#include <iostream>
#include <vector>

namespace FWA::SCANNER {
void validateChannelNumbers(const std::set<int> &channelNumbers,
                            const std::string &channelType) {
  if (channelNumbers.empty()) {
    std::cout << channelType << " channel numbers: None found" << std::endl;
    return;
  }

  // Check if the numbers are sequential
  int expectedNext = *channelNumbers.begin();
  bool sequential = true;
  std::vector<int> gaps;

  for (int num : channelNumbers) {
    if (num != expectedNext) {
      sequential = false;
      // Record the gap
      while (expectedNext < num) {
        gaps.push_back(expectedNext);
        expectedNext++;
      }
    }
    expectedNext = num + 1;
  }

  // Report findings
  std::cout << channelType << " channel numbers: ";
  if (sequential) {
    std::cout << "Sequential from " << *channelNumbers.begin() << " to "
              << *channelNumbers.rbegin() << " (" << channelNumbers.size()
              << " channels)" << std::endl;
  } else {
    std::cout << "Non-sequential with gaps: ";
    for (int gap : gaps)
      std::cout << gap << " ";
    std::cout << std::endl;
    std::cout << "Found " << channelNumbers.size() << " channels, from "
              << *channelNumbers.begin() << " to " << *channelNumbers.rbegin()
              << std::endl;
  }

  // Check if the numbers are within reasonable limits
  const int MAX_REASONABLE_CHANNELS = 64; // Adjust based on device capabilities
  if (*channelNumbers.rbegin() > MAX_REASONABLE_CHANNELS) {
    std::cout << "Warning: " << channelType
              << " channel numbers exceed reasonable limit of "
              << MAX_REASONABLE_CHANNELS << std::endl;
  }
}

} // namespace FWA::SCANNER
