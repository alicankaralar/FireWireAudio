#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <set>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER
{
	// --- String Extraction Utility ---

	/**
	 * @brief Represents a string found in memory with its address and type
	 */
	struct StringMatch
	{
		std::string text; ///< The extracted string text
		uint64_t address; ///< The memory address where the string was found
		bool isByteLevel; ///< Whether the string was found at the byte level or quadlet level
	};

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
	std::vector<StringMatch> extractStringsFromMemory(const std::map<uint64_t, uint32_t> &registers);

	/**
	 * @brief Dynamically discovers the address where channel names are stored
	 *
	 * This function tries several potential addresses where channel names might be stored
	 * and looks for patterns like "OUTPUT CH1" or "INPUT CH1" to identify the correct address.
	 *
	 * @param deviceInterface Pointer-to-pointer to an opened IOFireWireDeviceInterface
	 * @param service The io_service_t representing the FireWire nub
	 * @param generation The current bus generation number
	 * @return The discovered address, or a default fallback if none is found
	 */
	uint64_t discoverChannelNamesAddress(IOFireWireDeviceInterface **deviceInterface,
																			 io_service_t service,
																			 UInt32 generation);

	/**
	 * @brief Validates a set of channel numbers for consistency and reasonableness
	 *
	 * Checks if channel numbers are sequential and within reasonable limits.
	 * Reports gaps in the sequence and warns if numbers exceed expected maximums.
	 *
	 * @param channelNumbers Set of channel numbers to validate
	 * @param channelType String describing the type of channels (e.g., "Output", "Input")
	 */
	void validateChannelNumbers(const std::set<int> &channelNumbers, const std::string &channelType);

} // namespace FWA::SCANNER

#endif // UTILS_STRING_HPP