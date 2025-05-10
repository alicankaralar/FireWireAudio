#ifndef UTILS_EXPLORE_HPP
#define UTILS_EXPLORE_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER {
// --- Utility and Debugging Functions ---

/**
 * @brief Performs a diagnostic scan around the standard DICE register base
 * address. Reads quadlets in a defined range and attempts to identify potential
 * version registers, coherent ASCII strings (like channel names), and
 * structured numeric data (like zero blocks). This is purely for debugging and
 * understanding memory layout.
 *
 * @param deviceInterface Pointer-to-pointer to an opened
 * IOFireWireDeviceInterface.
 * @param service The io_service_t representing the FireWire nub.
 * @param device Reference to the FireWireDevice struct (used for context, not
 * modified).
 * @param generation The current bus generation number.
 * @param baseAddr The base address to explore around.
 */
void exploreDiceMemoryLayout(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt32 generation, UInt64 baseAddr);

/**
 * @brief Extracts coherent ASCII strings from DICE register values.
 * Analyzes the register values stored in the device struct to find and
 * combine meaningful ASCII strings that may span across multiple registers.
 * Organizes output by address ranges and string types.
 *
 * @param device The FireWireDevice struct containing the register values to
 * analyze.
 */
void extractCoherentRegisterStrings(const FireWireDevice &device);

/**
 * @brief Performs a detailed scan of the output channel names area.
 * Scans a larger range around the known output channel names address
 * to find all channel names and related strings.
 *
 * @param deviceInterface The FireWire device interface.
 * @param service The IO service.
 * @param device The FireWireDevice struct to store the results.
 * @param generation The FireWire bus generation.
 */
void exploreChannelNamesArea(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt32 generation);

} // namespace FWA::SCANNER

#endif // UTILS_EXPLORE_HPP
