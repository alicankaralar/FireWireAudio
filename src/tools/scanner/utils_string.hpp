#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

// Include the new modular header files
#include "channel_discovery.hpp"
#include "string_extraction.hpp"
#include "validation_utils.hpp"

// This header now serves as an aggregation point for string-related utilities
// All functionality has been moved to more focused modules

namespace FWA::SCANNER {
// All functionality has been moved to the following modules:
// - string_extraction.hpp: StringMatch struct, extractStringsFromMemory,
// extractPrintableStringsFromBytes
// - channel_discovery.hpp: discoverChannelNamesAddress
// - validation_utils.hpp: validateChannelNumbers

// This header is kept for backward compatibility

} // namespace FWA::SCANNER

#endif // UTILS_STRING_HPP
