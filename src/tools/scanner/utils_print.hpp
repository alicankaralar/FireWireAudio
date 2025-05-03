#ifndef UTILS_PRINT_HPP
#define UTILS_PRINT_HPP

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER
{
	/**
	 * @brief Prints the collected information about a FireWire device to standard output.
	 * Includes GUID, Name, Vendor, detected DICE info (Chip Type, Config, Streams),
	 * and a formatted list of read DICE registers.
	 *
	 * @param device The FireWireDevice struct containing the information to print.
	 */
	void printDeviceInfo(const FireWireDevice &device);

} // namespace FWA::SCANNER

#endif // UTILS_PRINT_HPP