#ifndef REGISTER_KNOWN_LOCATIONS_HPP
#define REGISTER_KNOWN_LOCATIONS_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Checks known locations for strings in DICE registers
 *
 * Examines predefined memory addresses where strings are commonly found
 * in DICE devices, such as device names and channel configurations.
 *
 * @param device The FireWireDevice struct containing register values
 * @return A map of register addresses to register values for further analysis
 */
std::map<uint64_t, uint32_t>
checkKnownStringLocations(const FireWireDevice &device);

} // namespace FWA::SCANNER

#endif // REGISTER_KNOWN_LOCATIONS_HPP
