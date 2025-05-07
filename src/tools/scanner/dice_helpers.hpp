#ifndef DICE_HELPERS_HPP
#define DICE_HELPERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

// Include the new header files that replace functionality from this file
#include "dice_base_discovery.hpp"
#include "dice_config.hpp"
#include "dice_register_readers.hpp"

namespace FWA::SCANNER {
// All functionality has been moved to the following files:
// - dice_base_discovery.hpp/cpp: Base address discovery
// - dice_register_readers.hpp/cpp: Register reading functions
// - dice_config.hpp/cpp: Configuration and main register reading
// - dice_stream_registers.hpp/cpp: Stream register reading
// - eap_helpers.hpp/cpp: EAP functionality

// This header is kept for backward compatibility
// Use the specific headers for new code

} // namespace FWA::SCANNER

#endif // DICE_HELPERS_HPP
