#ifndef UTILS_EXPLORE_CHANNELS_HPP
#define UTILS_EXPLORE_CHANNELS_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER
{
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
															 io_service_t service,
															 FireWireDevice &device,
															 UInt32 generation);

} // namespace FWA::SCANNER

#endif // UTILS_EXPLORE_CHANNELS_HPP