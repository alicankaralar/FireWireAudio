#ifndef UTILS_MEMORY_HPP
#define UTILS_MEMORY_HPP

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp"      // For FireWireDevice struct
#include "utils_string.hpp" // Include for StringMatch definition

namespace FWA::SCANNER {
// --- String Extraction Utility ---

/**
 * @brief Represents a string found in memory with its address and type
 */
// StringMatch struct definition moved to utils_string.hpp to avoid redefinition

/**
 * @brief Extracts strings from a map of register values
 *
 * This function analyzes register values to find both quadlet-level strings
 * (where each register contains a complete ASCII string) and byte-level strings
 * (where ASCII characters are spread across individual bytes in registers).
 *
 * @param registers Map of register addresses to register values
 * @return Vector of StringMatch objects containing the extracted strings
 */
std::vector<StringMatch>
extractStringsFromMemory(const std::map<uint64_t, uint32_t> &registers);

/**
 * @brief Dynamically discovers the address where channel names are stored
 *
 * This function tries several potential addresses where channel names might be
 * stored and looks for patterns like "OUTPUT CH1" or "INPUT CH1" to identify
 * the correct address.
 *
 * @param deviceInterface Pointer-to-pointer to an opened
 * IOFireWireDeviceInterface
 * @param service The io_service_t representing the FireWire nub
 * @param generation The current bus generation number
 * @return The discovered address, or a default fallback if none is found
 */
uint64_t
discoverChannelNamesAddress(IOFireWireDeviceInterface **deviceInterface,
                            io_service_t service, UInt32 generation);

/**
 * @brief Validates a set of channel numbers for consistency and reasonableness
 *
 * Checks if channel numbers are sequential and within reasonable limits.
 * Reports gaps in the sequence and warns if numbers exceed expected maximums.
 *
 * @param channelNumbers Set of channel numbers to validate
 * @param channelType String describing the type of channels (e.g., "Output",
 * "Input")
 */
void validateChannelNumbers(const std::set<int> &channelNumbers,
                            const std::string &channelType);

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
                             UInt32 generation, uint64_t baseAddr);

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

#endif // UTILS_MEMORY_HPP
