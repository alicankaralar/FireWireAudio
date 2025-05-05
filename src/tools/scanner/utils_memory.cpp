#include "utils_memory.hpp"
#include "io_helpers.hpp" // For safeReadQuadlet, interpretAsASCII
#include "scanner.hpp"    // For FireWireDevice, DiceDefines.hpp constants

#include <algorithm> // For std::sort
#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <iostream>
#include <map>
#include <regex> // For std::regex, std::smatch
#include <set>   // For std::set
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost

namespace FWA::SCANNER {

// --- Utility and Debugging Functions Implementation ---
// Definitions removed from this file to resolve duplicate symbol errors.
// They belong in utils_string.cpp, utils_explore_general.cpp, or
// utils_explore_channels.cpp.

} // namespace FWA::SCANNER
