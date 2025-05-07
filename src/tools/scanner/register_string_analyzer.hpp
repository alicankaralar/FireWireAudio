#ifndef REGISTER_STRING_ANALYZER_HPP
#define REGISTER_STRING_ANALYZER_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "register_address_ranges.hpp"
#include "register_known_locations.hpp"
#include "register_name_analyzer.hpp"
#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
/**
 * @brief Extracts coherent ASCII strings from DICE register values.
 * Analyzes the register values stored in the device struct to find and
 * combine meaningful ASCII strings that may span across multiple registers.
 * Organizes output by address ranges and string types.
 *
 * This function performs several types of analysis:
 * 1. Groups registers by address proximity
 * 2. Extracts quadlet-level strings (where entire quadlets form ASCII)
 * 3. Extracts byte-level strings (where individual bytes form ASCII)
 * 4. Looks for specific string types (device names, channel names, clock
 * sources)
 *
 * @param device The FireWireDevice struct containing the register values to
 * analyze.
 */
void extractCoherentRegisterStrings(const FireWireDevice &device);

} // namespace FWA::SCANNER

#endif // REGISTER_STRING_ANALYZER_HPP
