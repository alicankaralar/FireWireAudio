#ifndef REGISTER_NAME_ANALYZER_HPP
#define REGISTER_NAME_ANALYZER_HPP

#include <cstdint>
#include <map>
#include <string>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Analyzes registers for channel and device names
 *
 * Examines registers to find TX/RX channel names, device nicknames,
 * and other device-specific string information.
 *
 * @param device The FireWireDevice struct containing register values
 */
void analyzeChannelAndDeviceNames(const FireWireDevice &device);

/**
 * @brief Analyzes registers for clock source names
 *
 * Examines registers to find clock source names and their
 * associated addresses.
 *
 * @param device The FireWireDevice struct containing register values
 */
void analyzeClockSourceNames(const FireWireDevice &device);

} // namespace FWA::SCANNER

#endif // REGISTER_NAME_ANALYZER_HPP
