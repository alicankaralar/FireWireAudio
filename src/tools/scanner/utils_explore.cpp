#include "utils_explore.hpp"
#include "io_helpers.hpp"   // For safeReadQuadlet, interpretAsASCII
#include "scanner.hpp"      // For FireWireDevice, DiceDefines.hpp constants
#include "utils_string.hpp" // For StringMatch, extractStringsFromMemory, validateChannelNumbers, discoverChannelNamesAddress

#include <algorithm> // For std::sort
#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <iostream>
#include <map>
#include <regex> // For std::regex, std::smatch
#include <set>   // For std::set
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost, CFSwapInt32BigToHost

namespace FWA::SCANNER {

// --- Utility and Debugging Functions Implementation ---
// Definitions for exploreDiceMemoryLayout, extractCoherentRegisterStrings,
// and exploreChannelNamesArea have been removed from this file
// as they belong in utils_explore_general.cpp and utils_explore_channels.cpp
// respectively.

} // namespace FWA::SCANNER
